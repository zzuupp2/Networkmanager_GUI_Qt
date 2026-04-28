#include <QDBusPendingReply>
#include <QDebug>

#include "connection_manager.h"

namespace Net {

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{
    // ⭐ 只在这里监听一次 activeConnectionAdded
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionAdded,
            this,
            [this](const QString &path) {

        auto ac = NetworkManager::findActiveConnection(path);
        if (!ac)
            return;

        QString uuid = ac->uuid();

        qDebug() << "🔥 ACTIVE:" << uuid;

        if (!m_savedDevAutoConnect.contains(uuid))
            return;

        // ===== 恢复 autoconnect =====
        for (auto it = m_savedDevAutoConnect[uuid].begin();
             it != m_savedDevAutoConnect[uuid].end(); ++it) {

            auto dev = NetworkManager::findNetworkInterface(it.key());
            if (dev) {
                qDebug() << "restore autoconnect:" << it.value();
                dev->setAutoconnect(it.value());
            }
        }

        m_savedDevAutoConnect.remove(uuid);
    });
}

// ======================= 配置管理 =======================

void ConnectionManager::addConnection(const NMVariantMapMap &settings)
{
    auto reply = NetworkManager::addConnection(settings);
    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *w) {
                QDBusPendingReply<QDBusObjectPath> r = *w;
                w->deleteLater();

                if (r.isError()) {
                    emit errorOccurred(QString(), r.error().message());
                    emit operationCompleted(QString(), false);
                    return;
                }

                const QString path = r.value().path();
                auto conn = NetworkManager::findConnection(path);
                const QString uuid = conn ? conn->uuid() : QString();

                emit connectionAdded(uuid);
                emit operationCompleted(uuid, true);
            });
}

void ConnectionManager::updateConnection(const QString &uuid, const NMVariantMapMap &newSettings)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        emit operationCompleted(uuid, false);
        return;
    }

    auto reply = conn->update(newSettings);
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

ConnectionSettingInfo ConnectionManager::getConnectionSettingInfo(const QString &uuid) const
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

    return ConnectionSettingInfo::fromNMSettings(conn->settings(), active);
}

// ======================= 激活/断开 =======================

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

    qDebug() << "Activating on device:" << selectedDev->interfaceName();

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

                qDebug() << "Activation request sent OK";

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

    // ⭐ 防止重复断开（关键）
    if (m_savedDevAutoConnect.contains(uuid)) {
        qDebug() << "Already deactivating, skip";
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
    qDebug() << "devUnis:" << devUnis;

    // ===== 保存 + 关闭 autoconnect =====
    for (const auto &uni : devUnis) {

        auto dev = NetworkManager::findNetworkInterface(uni);
        if (!dev)
            continue;

        bool originalAuto = dev->autoconnect();
        qDebug() << "original:" << originalAuto;

        // ⭐ 不再 append，而是覆盖（防叠加）
        m_savedDevAutoConnect[uuid][uni] = originalAuto;

        dev->setAutoconnect(false);
    }

    // ===== 异步断开 =====
    auto reply = NetworkManager::deactivateConnection(targetAc->path());
    auto watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, watcher, uuid](QDBusPendingCallWatcher *w) {

                QDBusPendingReply<> r = *w;
                w->deleteLater();

                if (r.isError()) {

                    // ===== 回滚 =====
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

void ConnectionManager::apply(const QVariantMap &settings, bool isNew, const QString &uuid)
{
    NMVariantMapMap nmSettings;
    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
        nmSettings.insert(it.key(), it.value().toMap());
    }

    if (isNew) {
        addConnection(nmSettings);
        return;
    }

    QString targetUuid = uuid;
    if (targetUuid.isEmpty()) {
        const auto connMap = settings.value("connection").toMap();
        targetUuid = connMap.value("uuid").toString();
    }

    if (targetUuid.isEmpty()) {
        emit errorOccurred(QString(), "Missing uuid for update");
        emit operationCompleted(QString(), false);
        return;
    }

    updateConnection(targetUuid, nmSettings);
}

void ConnectionManager::remove(const QString &uuid)
{
    deleteConnection(uuid);
}

void ConnectionManager::activate(const QString &uuid)
{
    activateConnection(uuid);
}

void ConnectionManager::deactivate(const QString &uuid)
{
    deactivateConnection(uuid);
}

// ======================= 私有辅助 =======================

QString ConnectionManager::connectionPath(const QString &uuid) const
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (conn && conn->isValid())
        return conn->path();
    return {};
}

} // namespace Net
