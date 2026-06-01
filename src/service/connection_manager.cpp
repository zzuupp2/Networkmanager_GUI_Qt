#include <QDBusPendingReply>

#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Ipv4Setting>

#include "connection_manager.h"
#include "src/utils/network_utils.h"

namespace Net {

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionAdded,
            this,
            [this](const QString &path) {

        auto ac = NetworkManager::findActiveConnection(path);
        if (!ac)
            return;

        QString uuid = ac->uuid();


        if (!m_savedDevAutoConnect.contains(uuid))
            return;

        for (auto it = m_savedDevAutoConnect[uuid].begin();
             it != m_savedDevAutoConnect[uuid].end(); ++it) {

            auto dev = NetworkManager::findNetworkInterface(it.key());
            if (dev) {
                dev->setAutoconnect(it.value());
            }
        }

        m_savedDevAutoConnect.remove(uuid);
    });
}

void ConnectionManager::addConnection(const NMVariantMapMap &settings)
{
    auto reply = NetworkManager::addConnection(settings);

    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<QDBusObjectPath> r = *w;
                w->deleteLater();

                if (r.isError()) {
                    emit errorOccurred({}, r.error().message());
                    emit operationCompleted({}, false);
                    return;
                }

                auto conn = NetworkManager::findConnection(r.value().path());
                QString uuid = conn ? conn->uuid() : QString();

                emit connectionAdded(uuid);
                emit operationCompleted(uuid, true);
            });
}

void ConnectionManager::updateConnection(
    const QString &uuid,
    const ConnectionSettingInfo &info)
{
    using R = NetworkManager::Setting;

    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        emit operationCompleted(uuid, false);
        return;
    }

    auto settings = conn->settings();
    if (!settings) {
        emit errorOccurred(uuid, "Invalid settings object");
        emit operationCompleted(uuid, false);
        return;
    }

    NMVariantMapMap appendSections; // settings 对象中不存在的 section 在此收集

    // 1. Connection（连接级属性在 ConnectionSettings 自身，非子 setting）
    if (!info.id.isEmpty())
        settings->setId(info.id);
    settings->setAutoconnect(info.autoconnect);
    settings->setAutoconnectPriority(info.autoconnectPriority);
    if (!info.interfaceName.isEmpty())
        settings->setInterfaceName(info.interfaceName);

    // 2. Wi-Fi
    if (info.type == "802-11-wireless") {

        auto w = settings->setting(R::Wireless)
                     .staticCast<NetworkManager::WirelessSetting>();

        if (w && !info.ssid.isEmpty()) {
            w->setSsid(info.ssid.toUtf8());
        }

        // security
        {
            auto sec = settings->setting(R::WirelessSecurity)
                           .staticCast<NetworkManager::WirelessSecuritySetting>();

            bool isNone = (info.wirelessSecurity.isEmpty() ||
                           info.wirelessSecurity == "none");
            bool hasPassword = !info.wirelessPassword.isEmpty();

            if (isNone || info.wirelessSecurity == "unknown") {
                if (sec) {
                    sec->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Unknown);
                    sec->setPsk(QString());
                    sec->setLeapUsername(QString());
                }
            } else if (sec) {
                auto km = NetUtils::stringToKeyMgmt(info.wirelessSecurity);

                sec->setKeyMgmt(km);

                bool isPskType =
                    (km == NetworkManager::WirelessSecuritySetting::WpaPsk ||
                     km == NetworkManager::WirelessSecuritySetting::SAE);
                sec->setPsk((isPskType && hasPassword) ? info.wirelessPassword : QString());

                if (km == NetworkManager::WirelessSecuritySetting::WpaEap &&
                    info.extra.contains("leap-username")) {
                    sec->setLeapUsername(info.extra["leap-username"].toString());
                } else {
                    sec->setLeapUsername(QString());
                }
            } else {
                QVariantMap secMap;
                secMap["key-mgmt"] = info.wirelessSecurity;

                auto km = NetUtils::stringToKeyMgmt(info.wirelessSecurity);
                bool isPskType =
                    (km == NetworkManager::WirelessSecuritySetting::WpaPsk ||
                     km == NetworkManager::WirelessSecuritySetting::SAE);
                if (isPskType && hasPassword)
                    secMap["psk"] = info.wirelessPassword;

                if (km == NetworkManager::WirelessSecuritySetting::WpaEap &&
                    info.extra.contains("leap-username"))
                    secMap["leap-username"] = info.extra["leap-username"];

                appendSections["802-11-wireless-security"] = secMap;
            }
        }
    }

    // 3. Ethernet
    if (info.type == "802-3-ethernet") {

        auto e = settings->setting(R::Wired)
                     .staticCast<NetworkManager::WiredSetting>();

        if (e) {
            e->setMtu(info.mtu);

            if (info.extra.contains("mac"))
                e->setMacAddress(info.extra["mac"].toString().toUtf8());

            if (info.wiredAutoNegotiate == 0) {
                e->setAutoNegotiate(false);
                if (info.wiredSpeed > 0)
                    e->setSpeed(static_cast<quint32>(info.wiredSpeed));
                if (!info.wiredDuplex.isEmpty()) {
                    e->setDuplexType(info.wiredDuplex == QStringLiteral("full")
                        ? NetworkManager::WiredSetting::Full
                        : NetworkManager::WiredSetting::Half);
                }
            } else if (info.wiredAutoNegotiate == 1) {
                e->setAutoNegotiate(true);
            }
            // -1（忽略）：不调用 setter
        }
    }

    // 4. IPv4
    {
        auto ipv4 = settings->setting(R::Ipv4)
                        .staticCast<NetworkManager::Ipv4Setting>();

        if (ipv4) {

            using M = NetworkManager::Ipv4Setting;
            M::ConfigMethod method = M::Automatic;

            if (info.ipv4Method == "manual") {
                method = M::Manual;
            } else if (info.ipv4Method == "disabled") {
                method = M::Disabled;
            } else if (info.ipv4Method == "link-local") {
                method = M::LinkLocal;
            } else if (info.ipv4Method == "shared") {
                method = M::Shared;
            }

            ipv4->setMethod(method);

            if (method == M::Manual) {
                NetworkManager::IpAddress addr;

                if (!info.ipv4Address.isEmpty()) {
                    if (info.ipv4Address.contains('/')) {
                        auto p = info.ipv4Address.split('/');
                        addr.setIp(QHostAddress(p[0]));
                        addr.setPrefixLength(p[1].toInt());
                    } else {
                        addr.setIp(QHostAddress(info.ipv4Address));
                        addr.setPrefixLength(24);
                    }

                    if (!info.ipv4Gateway.isEmpty())
                        addr.setGateway(QHostAddress(info.ipv4Gateway));

                    ipv4->setAddresses({addr});
                }

                QList<QHostAddress> dns;
                for (const auto &d : info.ipv4Dns) {
                    if (!d.trimmed().isEmpty())
                        dns << QHostAddress(d.trimmed());
                }
                ipv4->setDns(dns);
            } else {
                ipv4->setAddresses({});
                ipv4->setDns({});
            }
        }
    }

    // 5. 合并新 section + 提交 update
    NMVariantMapMap finalMap = settings->toMap();
    for (auto it = appendSections.begin(); it != appendSections.end(); ++it) {
        finalMap.insert(it.key(), it.value());
    }


    auto reply = conn->update(finalMap);

    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, uuid](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<> r = *w;
                w->deleteLater();

                if (r.isError()) {
                    emit errorOccurred(uuid, r.error().message());
                    return;
                }

                emit connectionUpdated(uuid);
                emit operationCompleted(uuid, true);
            });
}

void ConnectionManager::deleteConnection(const QString &uuid)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        emit operationCompleted(uuid, false);
        return;
    }

    auto reply = conn->remove();
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, uuid](QDBusPendingCallWatcher *w) {
                QDBusPendingReply<> r = *w;
                w->deleteLater();

                if (r.isError()) {
                    emit errorOccurred(uuid, r.error().message());
                    emit operationCompleted(uuid, false);
                    return;
                }

                emit connectionRemoved(uuid);
                emit operationCompleted(uuid, true);
            });
}

ConnectionSettingInfo ConnectionManager::getConnectionSettingInfo(const QString &uuid)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        return ConnectionSettingInfo();
    }

    bool active = false;
    // 检查是否处于活跃状态
    for (const auto &acon : NetworkManager::activeConnections()) {
        if (acon->connection() && acon->connection()->uuid() == uuid) {
            active = true;
            break;
        }
    }

    auto info = ConnectionSettingInfo::fromNMSettings(conn->settings(), active);

    // WiFi 密码不会通过 settings() 返回（NM 安全策略），需要单独请求 secrets
    if (info.typeEnum == NetworkManager::ConnectionSettings::Wireless &&
        info.wirelessPassword.isEmpty()) {
        QDBusPendingReply<NMVariantMapMap> reply =
            conn->secrets("802-11-wireless-security");
        reply.waitForFinished();
        if (!reply.isError()) {
            auto secMap = reply.value().value("802-11-wireless-security");
            info.wirelessPassword = secMap.value("psk").toString();
        }
    }

    return info;
}

void ConnectionManager::addAndActivateConnection(
    const NMVariantMapMap &settings)
{
    auto reply = NetworkManager::addConnection(settings);

    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<QDBusObjectPath> r = *w;

                w->deleteLater();

                if (r.isError()) {

                    emit errorOccurred(
                        QString(),
                        r.error().message());

                    emit operationCompleted(QString(), false);

                    return;
                }

                QString path = r.value().path();

                auto conn = NetworkManager::findConnection(path);

                if (!conn) {

                    emit operationCompleted(QString(), false);

                    return;
                }

                QString uuid = conn->uuid();

                emit connectionAdded(uuid);

                activateConnection(uuid);

                emit operationCompleted(uuid, true);
            });
}

void ConnectionManager::createAndActivateWifiConnection(
    const QString &ssid,
    const QString &password,
    const QString &security,
    const QString &iface)
{
    if (ssid.isEmpty()) {

        emit errorOccurred(
            QString(),
            "SSID is empty");

        return;
    }

    NMVariantMapMap settings;

    // connection
    QVariantMap connection;

    connection["id"] = ssid;

    connection["uuid"] =
        QUuid::createUuid()
            .toString(QUuid::WithoutBraces);

    connection["type"] = "802-11-wireless";

    connection["autoconnect"] = true;

    if (!iface.isEmpty())
        connection["interface-name"] = iface;

    settings["connection"] = connection;

    // wifi
    QVariantMap wifi;

    wifi["ssid"] = ssid.toUtf8();

    wifi["mode"] = "infrastructure";

    settings["802-11-wireless"] = wifi;

    // wifi-security
    if (!password.isEmpty()) {

        QVariantMap wifiSec;

        wifiSec["key-mgmt"] = security;

        wifiSec["psk"] = password;

        settings["802-11-wireless-security"] =
            wifiSec;
    }

    // ipv4
    QVariantMap ipv4;

    ipv4["method"] = "auto";

    settings["ipv4"] = ipv4;

    // ipv6
    QVariantMap ipv6;

    ipv6["method"] = "auto";

    settings["ipv6"] = ipv6;

    // create + activate
    addAndActivateConnection(settings);
}

void ConnectionManager::activateConnection(const QString &uuid)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        emit operationCompleted(uuid, false);
        return;
    }

    auto settings = conn->settings();
    NetworkManager::Device::Ptr selectedDev;
    const QString preferredIface = settings ? settings->interfaceName() : QString();

    auto matchesConnectionType = [settings](const NetworkManager::Device::Ptr &dev) {
        if (!settings || !dev)
            return false;

        if (settings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
            return dev->type() == NetworkManager::Device::Wifi;
        }

        if (settings->connectionType() == NetworkManager::ConnectionSettings::Wired) {
            return dev->type() == NetworkManager::Device::Ethernet;
        }

        return false;
    };

    // 优先匹配 connection.interface-name
    if (!preferredIface.isEmpty()) {
        for (const auto &dev : NetworkManager::networkInterfaces()) {
            if (matchesConnectionType(dev) && dev->interfaceName() == preferredIface) {
                selectedDev = dev;
                break;
            }
        }
    }

    // 否则回退为同类型第一个可用设备
    if (!selectedDev) {
        for (const auto &dev : NetworkManager::networkInterfaces()) {
            if (matchesConnectionType(dev)) {
                if (dev->state() == NetworkManager::Device::Unavailable)
                    continue;

                selectedDev = dev;
                break;
            }
        }
    }

    if (!selectedDev) {
        // 兜底：允许 Unavailable 也尝试一次
        for (const auto &dev : NetworkManager::networkInterfaces()) {
            if (matchesConnectionType(dev)) {
                selectedDev = dev;
                break;
            }
        }
    }

    if (!selectedDev) {
        emit errorOccurred(uuid, "No suitable device");
        emit operationCompleted(uuid, false);
        return;
    }

    auto reply = NetworkManager::activateConnection(
        conn->path(),
        selectedDev->uni(),
        QString()
        );

    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, uuid](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<QDBusObjectPath> r = *w;
                w->deleteLater();

                if (r.isError()) {
                    emit errorOccurred(uuid, r.error().message());
                    emit operationCompleted(uuid, false);
                    return;
                }


                emit operationCompleted(uuid, true);
            });

}

void ConnectionManager::deactivateConnection(const QString &uuid)
{
    if (uuid.isEmpty()) {
        emit errorOccurred(uuid, "UUID is empty");
        emit operationCompleted(uuid, false);
        return;
    }

    if (m_savedDevAutoConnect.contains(uuid)) {
        emit errorOccurred(uuid, "Deactivation already in progress");
        emit operationCompleted(uuid, false);
        return;
    }

    NetworkManager::ActiveConnection::Ptr targetAc;

    for (const auto &acon : NetworkManager::activeConnections()) {
        if (acon->connection() && acon->connection()->uuid() == uuid) {
            targetAc = acon;
            break;
        }
    }

    if (!targetAc) {
        emit errorOccurred(uuid, "No active connection with this UUID");
        emit operationCompleted(uuid, false);
        return;
    }

    const QStringList devUnis = targetAc->devices();

    for (const auto &uni : devUnis) {

        auto dev = NetworkManager::findNetworkInterface(uni);
        if (!dev)
            continue;

        bool originalAuto = dev->autoconnect();
        m_savedDevAutoConnect[uuid][uni] = originalAuto;

        dev->setAutoconnect(false);
    }

    auto reply = NetworkManager::deactivateConnection(targetAc->path());
    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, uuid](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<> r = *w;
                w->deleteLater();

                if (r.isError()) {

                    if (m_savedDevAutoConnect.contains(uuid)) {

                        for (auto it = m_savedDevAutoConnect[uuid].begin();
                             it != m_savedDevAutoConnect[uuid].end(); ++it) {

                            auto dev = NetworkManager::findNetworkInterface(it.key());
                            if (dev)
                                dev->setAutoconnect(it.value());
                        }

                        m_savedDevAutoConnect.remove(uuid);
                    }

                    emit errorOccurred(uuid, r.error().message());
                    emit operationCompleted(uuid, false);
                    return;
                }

                emit operationCompleted(uuid, true);
            });
}

void ConnectionManager::apply(
    const ConnectionSettingInfo &info,
    bool isNew,
    const QString &uuid)
{
    const QString targetUuid = uuid.isEmpty() ? info.uuid : uuid;

    if (isNew) {
        addConnection(info.toNMSettings());
        return;
    }

    if (targetUuid.isEmpty()) {
        emit errorOccurred({}, "Missing uuid");
        return;
    }

    updateConnection(targetUuid, info);
}

} // namespace Net
