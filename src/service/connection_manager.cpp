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

QString ConnectionManager::addConnection(const NMVariantMapMap &settings)
{
    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(settings);
    reply.waitForFinished();

    if (reply.isError()) {
        emit errorOccurred(QString(), reply.error().message());
        return QString();
    }

    // 从返回的路径中提取 UUID
    QString path = reply.value().path();
    NetworkManager::Connection::Ptr conn = NetworkManager::findConnection(path);
    if (!conn) {
        emit errorOccurred(QString(), "Connection created but unable to find it");
        return QString();
    }

    QString uuid = conn->uuid();
    emit connectionAdded(uuid);
    return uuid;
}

bool ConnectionManager::updateConnection(const QString &uuid, const NMVariantMapMap &newSettings)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        return false;
    }

    // 更新连接
    QDBusPendingReply<> reply = conn->update(newSettings);
    reply.waitForFinished();

    if (reply.isError()) {
        emit errorOccurred(uuid, reply.error().message());
        return false;
    }

    emit connectionUpdated(uuid);
    return true;
}

bool ConnectionManager::deleteConnection(const QString &uuid)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        return false;
    }

    QDBusPendingReply<> reply = conn->remove();
    reply.waitForFinished();

    if (reply.isError()) {
        emit errorOccurred(uuid, reply.error().message());
        return false;
    }

    emit connectionRemoved(uuid);
    return true;
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

bool ConnectionManager::activateConnection(const QString &uuid)
{
    auto conn = NetworkManager::findConnectionByUuid(uuid);
    if (!conn || !conn->isValid()) {
        emit errorOccurred(uuid, "Connection not found");
        return false;
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
        return false;
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
                    return;
                }

                qDebug() << "Activation request sent OK";

                emit operationCompleted(uuid, true);
            });

    return true;
}

bool ConnectionManager::deactivateConnection(const QString &uuid)
{
    if (uuid.isEmpty()) {
        emit errorOccurred(uuid, "UUID is empty");
        return false;
    }

    // ⭐ 防止重复断开（关键）
    if (m_savedDevAutoConnect.contains(uuid)) {
        qDebug() << "Already deactivating, skip";
        return false;
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
        return false;
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
                    return;
                }

                emit operationCompleted(uuid, true);
            });

    return true;
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
