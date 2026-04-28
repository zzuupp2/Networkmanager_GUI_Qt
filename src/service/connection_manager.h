#pragma once

#include <QObject>
#include <QHash>
#include <QVariantMap>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Settings>

#include "src/model/connection/con_setting_info.h"

namespace Net {

class ConnectionManager : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionManager(QObject *parent = nullptr);

    // ====== 配置管理 ======
    Q_INVOKABLE QString addConnection(const NMVariantMapMap &settings);
    Q_INVOKABLE bool updateConnection(const QString &uuid, const NMVariantMapMap &newSettings);
    Q_INVOKABLE bool deleteConnection(const QString &uuid);
    ConnectionSettingInfo getConnectionSettingInfo(const QString &uuid) const;

    // ====== 激活/断开 ======
    Q_INVOKABLE bool activateConnection(const QString &uuid);
    Q_INVOKABLE bool deactivateConnection(const QString &uuid);

    // ====== 简化命名（供 QML/Wrapper 使用） ======
    Q_INVOKABLE QString apply(const QVariantMap &settings, bool isNew, const QString &uuid = QString());
    Q_INVOKABLE bool remove(const QString &uuid);
    Q_INVOKABLE bool activate(const QString &uuid);
    Q_INVOKABLE bool deactivate(const QString &uuid);

signals:
    void connectionAdded(const QString &uuid);
    void connectionRemoved(const QString &uuid);
    void connectionUpdated(const QString &uuid);
    void operationCompleted(const QString &uuid, bool success);
    void errorOccurred(const QString &uuid, const QString &error);

private:
    // QHash<QString, QPair<QString, bool>> m_savedDevAutoConnect;
    // QHash<QString, QList<QPair<QString, bool>>> m_savedDevAutoConnect;
    QHash<QString, QHash<QString, bool>> m_savedDevAutoConnect;
    QString connectionPath(const QString &uuid) const;
    // NetworkManager::Device::Ptr deviceForActiveConnection(const QString &devUuid) const;
    // void setDeviceAutoconnect(const QString &uuid, bool setEnabled);
};

} // namespace Net
