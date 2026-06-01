#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <NetworkManagerQt/ConnectionSettings>

namespace Net {

struct ConnectionSettingInfo
{
    QString id;
    QString uuid;
    QString type;
    NetworkManager::ConnectionSettings::ConnectionType typeEnum;

    QDateTime timestamp;
    bool autoconnect = true;
    int autoconnectPriority = 0;
    bool active = false;

    QString interfaceName;

    QString ssid;
    QString wirelessSecurity;
    QString wirelessPassword;

    int mtu = 0;
    int wiredAutoNegotiate = -1;   // -1=ignore, 0=manual, 1=auto
    int wiredSpeed = 0;            // Mb/s, 0=unset
    QString wiredDuplex;           // "full" / "half" / ""

    QString ipv4Method;
    QString ipv4Address;
    QString ipv4Gateway;
    QStringList ipv4Dns;

    QVariantMap extra;

    // NM -> Info（唯一入口）
    static ConnectionSettingInfo fromNMSettings(
        const NetworkManager::ConnectionSettings::Ptr &settings,
        bool active = false);

    NMVariantMapMap toNMSettings() const;
};

} // namespace Net

Q_DECLARE_METATYPE(Net::ConnectionSettingInfo)