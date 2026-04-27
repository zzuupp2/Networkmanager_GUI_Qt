#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include "con_setting_info.h"

namespace Net {

class ConnectionEditorModel : public QObject
{
    Q_OBJECT

    // ========== 通用标识（只读或可编辑） ==========
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)                       // 不可编辑
    Q_PROPERTY(QString type READ type CONSTANT)                       // 连接类型（只读）
    Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY autoconnectChanged)
    Q_PROPERTY(int autoconnectPriority READ autoconnectPriority WRITE setAutoconnectPriority NOTIFY autoconnectPriorityChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)

    // ========== 无线 (Wi-Fi) 专用 ==========
    Q_PROPERTY(QString ssid READ ssid WRITE setSsid NOTIFY ssidChanged)
    Q_PROPERTY(QString wirelessSecurity READ wirelessSecurity WRITE setWirelessSecurity NOTIFY wirelessSecurityChanged)
    Q_PROPERTY(QString wirelessPassword READ wirelessPassword WRITE setWirelessPassword NOTIFY wirelessPasswordChanged)

    // ========== 有线专用 ==========
    Q_PROPERTY(int mtu READ mtu WRITE setMtu NOTIFY mtuChanged)

    // ========== IPv4 设置 ==========
    Q_PROPERTY(QString ipv4Method READ ipv4Method WRITE setIpv4Method NOTIFY ipv4MethodChanged)
    Q_PROPERTY(QString ipv4Address READ ipv4Address WRITE setIpv4Address NOTIFY ipv4AddressChanged)
    Q_PROPERTY(QString ipv4Gateway READ ipv4Gateway WRITE setIpv4Gateway NOTIFY ipv4GatewayChanged)
    Q_PROPERTY(QStringList ipv4Dns READ ipv4Dns WRITE setIpv4Dns NOTIFY ipv4DnsChanged)

    // // ========== IPv6 设置（可选） ==========
    // Q_PROPERTY(QString ipv6Method READ ipv6Method WRITE setIpv6Method NOTIFY ipv6MethodChanged)
    // Q_PROPERTY(QString ipv6Address READ ipv6Address WRITE setIpv6Address NOTIFY ipv6AddressChanged)
    // Q_PROPERTY(QString ipv6Gateway READ ipv6Gateway WRITE setIpv6Gateway NOTIFY ipv6GatewayChanged)
    // Q_PROPERTY(QStringList ipv6Dns READ ipv6Dns WRITE setIpv6Dns NOTIFY ipv6DnsChanged)

    // ========== 编辑状态 ==========
    Q_PROPERTY(bool isModified READ isModified NOTIFY isModifiedChanged)
    Q_PROPERTY(bool isNew READ isNew CONSTANT)                        // 是否新建连接

public:
    explicit ConnectionEditorModel(QObject *parent = nullptr);

    // ----- 数据加载 -----
    void loadFromSettingInfo(const ConnectionSettingInfo &info);
    void loadDefaults(const QString &type);          // 新建连接时调用，如 "802-11-wireless"

    // ----- 操作 -----
    Q_INVOKABLE void reset();                        // 取消编辑，恢复到原始值
    Q_INVOKABLE ConnectionSettingInfo toSettingInfo() const;      // 导出为结构体
    Q_INVOKABLE QVariantMap toSettingsMap() const;                // 导出为 NM 需要的 QVariantMap

    // ----- 属性访问（各 getter/setter） -----
    QString id() const;
    void setId(const QString &id);
    QString uuid() const;
    QString type() const;

    bool autoconnect() const;
    void setAutoconnect(bool enabled);
    int autoconnectPriority() const;
    void setAutoconnectPriority(int priority);
    QString interfaceName() const;
    void setInterfaceName(const QString &iface);

    QString ssid() const;
    void setSsid(const QString &ssid);
    QString wirelessSecurity() const;
    void setWirelessSecurity(const QString &security);
    QString wirelessPassword() const;
    void setWirelessPassword(const QString &password);

    int mtu() const;
    void setMtu(int mtu);

    QString ipv4Method() const;
    void setIpv4Method(const QString &method);
    QString ipv4Address() const;
    void setIpv4Address(const QString &address);
    QString ipv4Gateway() const;
    void setIpv4Gateway(const QString &gateway);
    QStringList ipv4Dns() const;
    void setIpv4Dns(const QStringList &dns);

    // QString ipv6Method() const;
    // void setIpv6Method(const QString &method);
    // QString ipv6Address() const;
    // void setIpv6Address(const QString &address);
    // QString ipv6Gateway() const;
    // void setIpv6Gateway(const QString &gateway);
    // QStringList ipv6Dns() const;
    // void setIpv6Dns(const QStringList &dns);

    bool isModified() const;
    bool isNew() const;

signals:
    void idChanged();
    void autoconnectChanged();
    void autoconnectPriorityChanged();
    void interfaceNameChanged();
    void ssidChanged();
    void wirelessSecurityChanged();
    void wirelessPasswordChanged();
    void mtuChanged();
    void ipv4MethodChanged();
    void ipv4AddressChanged();
    void ipv4GatewayChanged();
    void ipv4DnsChanged();
    // void ipv6MethodChanged();
    // void ipv6AddressChanged();
    // void ipv6GatewayChanged();
    // void ipv6DnsChanged();
    void isModifiedChanged(bool modified);

    // 可选：通知外部操作状态
    void editAccepted();    // 保存成功后发射
    void editRejected();    // 取消时发射

private:
    void updateModificationFlag();

    ConnectionSettingInfo m_original;   // 加载时的原始数据
    ConnectionSettingInfo m_working;    // 工作副本，所有 getter 都读取这里
    bool m_isModified = false;
    bool m_isNew = false;
};

} // namespace Net