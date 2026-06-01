#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include "con_setting_info.h"

namespace Net {

class ConnectionManager;

class ConnectionEditorModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString uuid READ uuid NOTIFY uuidChanged)
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)

    Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY autoconnectChanged)
    Q_PROPERTY(int autoconnectPriority READ autoconnectPriority WRITE setAutoconnectPriority NOTIFY autoconnectPriorityChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)

    Q_PROPERTY(QString ssid READ ssid WRITE setSsid NOTIFY ssidChanged)
    Q_PROPERTY(QString wirelessSecurity READ wirelessSecurity WRITE setWirelessSecurity NOTIFY wirelessSecurityChanged)
    Q_PROPERTY(QString wirelessPassword READ wirelessPassword WRITE setWirelessPassword NOTIFY wirelessPasswordChanged)

    Q_PROPERTY(int mtu READ mtu WRITE setMtu NOTIFY mtuChanged)

    Q_PROPERTY(int wiredAutoNegotiate READ wiredAutoNegotiate WRITE setWiredAutoNegotiate NOTIFY wiredAutoNegotiateChanged)
    Q_PROPERTY(int wiredSpeed READ wiredSpeed WRITE setWiredSpeed NOTIFY wiredSpeedChanged)
    Q_PROPERTY(QString wiredDuplex READ wiredDuplex WRITE setWiredDuplex NOTIFY wiredDuplexChanged)

    Q_PROPERTY(QString ipv4Method READ ipv4Method WRITE setIpv4Method NOTIFY ipv4MethodChanged)
    Q_PROPERTY(QString ipv4Address READ ipv4Address WRITE setIpv4Address NOTIFY ipv4AddressChanged)
    Q_PROPERTY(QString ipv4Gateway READ ipv4Gateway WRITE setIpv4Gateway NOTIFY ipv4GatewayChanged)
    Q_PROPERTY(QStringList ipv4Dns READ ipv4Dns WRITE setIpv4Dns NOTIFY ipv4DnsChanged)

    Q_PROPERTY(bool isModified READ isModified NOTIFY isModifiedChanged)
    Q_PROPERTY(bool isNew READ isNew NOTIFY isNewChanged)                        // 是否新建连接

public:
    explicit ConnectionEditorModel(QObject *parent = nullptr);

    void loadFromSettingInfo(const ConnectionSettingInfo &info);
    Q_INVOKABLE void loadDefaults(const QString &type);
    void setConnectionManager(ConnectionManager *manager);
    bool loadByUuid(const QString &uuid);

    Q_INVOKABLE void reset();
    Q_INVOKABLE void commit();
    Q_INVOKABLE QVariantMap toPatch() const;
    Q_INVOKABLE ConnectionSettingInfo toSettingInfo() const;

    QString id() const;
    void setId(const QString &id);

    QString uuid() const;
    QString type() const;

    bool autoconnect() const;
    void setAutoconnect(bool v);

    int autoconnectPriority() const;
    void setAutoconnectPriority(int v);

    QString interfaceName() const;
    void setInterfaceName(const QString &v);

    QString ssid() const;
    void setSsid(const QString &v);

    QString wirelessSecurity() const;
    void setWirelessSecurity(const QString &v);

    QString wirelessPassword() const;
    void setWirelessPassword(const QString &v);

    int mtu() const;
    void setMtu(int v);

    int wiredAutoNegotiate() const;
    void setWiredAutoNegotiate(int v);

    int wiredSpeed() const;
    void setWiredSpeed(int v);

    QString wiredDuplex() const;
    void setWiredDuplex(const QString &v);

    QString ipv4Method() const;
    void setIpv4Method(const QString &v);

    QString ipv4Address() const;
    void setIpv4Address(const QString &v);

    QString ipv4Gateway() const;
    void setIpv4Gateway(const QString &v);

    QStringList ipv4Dns() const;
    void setIpv4Dns(const QStringList &v);

    bool isModified() const;
    bool isNew() const;

signals:
    void idChanged();
    void uuidChanged();
    void typeChanged();

    void autoconnectChanged();
    void autoconnectPriorityChanged();
    void interfaceNameChanged();

    void ssidChanged();
    void wirelessSecurityChanged();
    void wirelessPasswordChanged();

    void mtuChanged();

    void wiredAutoNegotiateChanged();
    void wiredSpeedChanged();
    void wiredDuplexChanged();

    void ipv4MethodChanged();
    void ipv4AddressChanged();
    void ipv4GatewayChanged();
    void ipv4DnsChanged();
    // void ipv6MethodChanged();
    // void ipv6AddressChanged();
    // void ipv6GatewayChanged();
    // void ipv6DnsChanged();
    void isModifiedChanged(bool modified);
    void isNewChanged(bool isNew);
    // 可选：通知外部操作状态
    void editAccepted();    // 保存成功后发射
    void editRejected();    // 取消时发射

private:
    void updateModificationFlag();

    ConnectionSettingInfo m_original;
    ConnectionSettingInfo m_working;

    ConnectionManager *m_manager = nullptr;

    bool m_isModified = false;
    bool m_isNew = false;
};

} // namespace Net