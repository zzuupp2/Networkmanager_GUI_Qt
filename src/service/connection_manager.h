#pragma once

#include <QObject>
#include <QHash>
#include <QUuid>
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

    Q_INVOKABLE void addConnection(const NMVariantMapMap &settings);
    Q_INVOKABLE void updateConnection(const QString &uuid, const ConnectionSettingInfo &info);
    Q_INVOKABLE void deleteConnection(const QString &uuid);
    Q_INVOKABLE void activateConnection(const QString &uuid);
    Q_INVOKABLE void deactivateConnection(const QString &uuid);
    ConnectionSettingInfo getConnectionSettingInfo(const QString &uuid);

    Q_INVOKABLE void createAndActivateWifiConnection(
        const QString &ssid,
        const QString &password,
        const QString &security = "wpa-psk",
        const QString &iface = QString());

    Q_INVOKABLE void apply(const ConnectionSettingInfo &info, bool isNew, const QString &uuid = QString());

signals:
    void connectionAdded(const QString &uuid);
    void connectionRemoved(const QString &uuid);
    void connectionUpdated(const QString &uuid);
    void operationCompleted(const QString &uuid, bool success);
    void errorOccurred(const QString &uuid, const QString &error);

private:
    QHash<QString, QHash<QString, bool>> m_savedDevAutoConnect;
    QString connectionPath(const QString &uuid) const;
};

} // namespace Net
