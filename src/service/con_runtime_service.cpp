#include "con_runtime_service.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/IpConfig>

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

    refreshActiveConnectionWatchers();
    updateAll();
}


RuntimeState ConnectionRuntimeService::state(const QString &uuid) const
{
    return m_states.value(uuid);
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

    QObject::connect(ac.data(), SIGNAL(stateChanged(uint,uint)),
                     this, SLOT(onWatchedActiveConnectionChanged()));
    QObject::connect(ac.data(), SIGNAL(stateChanged(uint,uint,uint)),
                     this, SLOT(onWatchedActiveConnectionChanged()));
    QObject::connect(ac.data(), SIGNAL(ipV4ConfigChanged()),
                     this, SLOT(onWatchedActiveConnectionChanged()));
    QObject::connect(ac.data(), SIGNAL(ipV6ConfigChanged()),
                     this, SLOT(onWatchedActiveConnectionChanged()));
    QObject::connect(ac.data(), SIGNAL(defaultChanged(bool)),
                     this, SLOT(onWatchedActiveConnectionChanged()));
}

void ConnectionRuntimeService::detachActiveConnection(const QString &path)
{
    auto it = m_watchedActiveConnections.find(path);
    if (it == m_watchedActiveConnections.end())
        return;

    QObject::disconnect(it.value().data(), nullptr, this, nullptr);
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

void ConnectionRuntimeService::onWatchedActiveConnectionChanged()
{
    updateAll();
}

void ConnectionRuntimeService::updateAll()
{
    refreshActiveConnectionWatchers();
    QHash<QString, RuntimeState> newStates;

    // ===== activating =====
    auto activating = NetworkManager::activatingConnection();
    if (activating && activating->isValid()) {

        const QString uuid = activating->uuid();
        RuntimeState &st = newStates[uuid];
        st.activating = true;

    }

    // ===== active =====
    for (const auto &ac : NetworkManager::activeConnections()) {

        if (!ac || !ac->isValid())
            continue;

        QString uuid = ac->uuid();
        RuntimeState st;
        st.active = true;

        auto ip4 = ac->ipV4Config();
        if (ip4.isValid()) {

            auto addrs = ip4.addresses();
            if (!addrs.isEmpty())
                st.ipv4 = addrs.first().ip().toString();

            st.gateway = ip4.gateway();

            for (const auto &addr : ip4.nameservers())
                st.dns << addr.toString();
        }

        if (newStates.contains(uuid))
            st.activating = newStates[uuid].activating;

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

    // 删除
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
