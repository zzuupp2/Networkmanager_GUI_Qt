#include "con_runtime_service.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/IpConfig>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/AccessPoint>

#include "src/utils/network_utils.h"

using namespace Net;

ConnectionRuntimeService::ConnectionRuntimeService(QObject *parent)
    : QObject(parent)
{
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionAdded,
            this,
            &ConnectionRuntimeService::onActiveConnectionAdded);

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionRemoved,
            this,
            &ConnectionRuntimeService::onActiveConnectionRemoved);

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::primaryConnectionChanged,
            this,
            &ConnectionRuntimeService::updateAll);

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activatingConnectionChanged,
            this,
            &ConnectionRuntimeService::updateAll);

    refreshActiveConnectionWatchers();
    updateAll();
}


RuntimeState ConnectionRuntimeService::state(const QString &uuid) const
{
    auto it = m_states.find(uuid);
    if (it != m_states.end())
        return *it;

    RuntimeState st;
    auto rit = m_stateReasons.find(uuid);
    if (rit != m_stateReasons.end())
        st.stateReason = *rit;
    return st;
}

void ConnectionRuntimeService::refreshActiveConnectionWatchers()
{
    QSet<QString> currentPaths;
    for (const auto &ac : NetworkManager::activeConnections()) {
        if (!ac || !ac->isValid())
            continue;
        currentPaths.insert(ac->path());
        if (!m_watchedActiveConnections.contains(ac->path()))
            attachActiveConnection(ac->path());
    }

    QStringList watchedPaths = m_watchedActiveConnections.keys();
    for (const auto &path : watchedPaths) {
        if (!currentPaths.contains(path))
            detachActiveConnection(path);
    }
}

void ConnectionRuntimeService::attachActiveConnection(const QString &path)
{
    auto ac = NetworkManager::findActiveConnection(path);
    if (!ac || !ac->isValid() || m_watchedActiveConnections.contains(path))
        return;

    m_watchedActiveConnections.insert(path, ac);

    QObject::connect(ac.data(),
                     &NetworkManager::ActiveConnection::stateChanged,
                     this,
                     [this] { updateAll(); });

    QObject::connect(ac.data(),
                     &NetworkManager::ActiveConnection::stateChangedReason,
                     this,
                     [this, uuid = ac->uuid()](NetworkManager::ActiveConnection::State state,
                                                NetworkManager::ActiveConnection::Reason reason) {
                         m_stateReasons[uuid] = static_cast<int>(reason);
                     });

    QObject::connect(ac.data(),
                     &NetworkManager::ActiveConnection::ipV4ConfigChanged,
                     this,
                     [this] { updateAll(); });
}

void ConnectionRuntimeService::detachActiveConnection(const QString &path)
{
    auto it = m_watchedActiveConnections.find(path);
    if (it == m_watchedActiveConnections.end())
        return;

    if (it.value()) {
        QObject::disconnect(it.value().data(), nullptr, this, nullptr);
    }
    m_watchedActiveConnections.erase(it);
}

void ConnectionRuntimeService::onActiveConnectionAdded(const QString &path)
{
    attachActiveConnection(path);
    updateAll();
}

void ConnectionRuntimeService::onActiveConnectionRemoved(const QString &path)
{
    detachActiveConnection(path);
    updateAll();
}

void ConnectionRuntimeService::updateAll()
{
    refreshActiveConnectionWatchers();
    QHash<QString, RuntimeState> newStates;

    auto activating = NetworkManager::activatingConnection();
    if (activating && activating->isValid()) {

        const QString uuid = activating->uuid();
        RuntimeState &st = newStates[uuid];
        st.activeState = static_cast<int>(activating->state());
        st.stateReason = m_stateReasons.value(uuid, 0);

    }

    for (const auto &ac : NetworkManager::activeConnections()) {

        if (!ac || !ac->isValid())
            continue;

        QString uuid = ac->uuid();
        RuntimeState st;
        st.activeState = static_cast<int>(ac->state());
        st.stateReason = m_stateReasons.value(uuid, 0);

        auto ip4 = ac->ipV4Config();
        if (ip4.isValid()) {

            auto addrs = ip4.addresses();
            if (!addrs.isEmpty()) {
                const auto &a = addrs.first();
                st.ipv4 = a.ip().toString() + "/" + QString::number(a.prefixLength());
            }

            st.gateway = ip4.gateway();

            for (const auto &addr : ip4.nameservers())
                st.dns << addr.toString();
        }

        const auto devPaths = ac->devices();
        if (!devPaths.isEmpty()) {
            auto dev = NetworkManager::findNetworkInterface(devPaths.first());
            if (dev) {
                st.interface = dev->interfaceName();
                st.mac = NetUtils::getHwAddr(dev);
                st.mtu = static_cast<int>(dev->mtu());

                auto conType = ac->type();
                if (conType == NetworkManager::ConnectionSettings::Wired) {
                    st.deviceType = QStringLiteral("wired");
                    if (auto wired = dev.objectCast<NetworkManager::WiredDevice>()) {
                        st.wiredSpeed = wired->bitRate();
                        st.carrier = wired->carrier();
                    }
                } else if (conType == NetworkManager::ConnectionSettings::Wireless) {
                    st.deviceType = QStringLiteral("wireless");
                    if (auto wifi = dev.objectCast<NetworkManager::WirelessDevice>()) {
                        st.wirelessRate = wifi->bitRate();
                        switch (wifi->mode()) {
                        case NetworkManager::WirelessDevice::Adhoc:
                            st.mode = QStringLiteral("Ad-Hoc"); break;
                        case NetworkManager::WirelessDevice::Infra:
                            st.mode = QStringLiteral("Infrastructure"); break;
                        case NetworkManager::WirelessDevice::ApMode:
                            st.mode = QStringLiteral("AP"); break;
                        default: break;
                        }

                        auto ap = wifi->activeAccessPoint();
                        if (ap) {
                            st.ssid = ap->ssid();
                            st.bandwidth = static_cast<int>(ap->bandwidth());

                            uint freq = ap->frequency();
                            if (freq >= 5900)
                                st.frequencyBand = QStringLiteral("6 GHz");
                            else if (freq >= 5000)
                                st.frequencyBand = QStringLiteral("5 GHz");
                            else if (freq >= 2000)
                                st.frequencyBand = QStringLiteral("2.4 GHz");
                        }
                    }
                }
            }
        }

        newStates.insert(uuid, st);
    }

    QSet<QString> changed;

    // 新增 / 更新
    for (auto it = newStates.begin(); it != newStates.end(); ++it) {
        const QString &uuid = it.key();

        if (!m_states.contains(uuid) || m_states[uuid] != it.value()) {
            m_states[uuid] = it.value();
            changed.insert(uuid);
        }
    }

    // 删除（保留 stateReason 以便外部查询）
    for (auto it = m_states.begin(); it != m_states.end();) {
        if (!newStates.contains(it.key())) {
            QString uuid = it.key();
            it = m_states.erase(it);
            changed.insert(uuid);
        } else {
            ++it;
        }
    }

    // 发信号
    for (const auto &uuid : changed)
        emit stateChanged(uuid);
}
