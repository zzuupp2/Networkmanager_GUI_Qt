// con_setting_info.h
#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <NetworkManagerQt/ConnectionSettings>

namespace Net {

struct ConnectionSettingInfo
{
    // ===== 通用标识 =====
    QString id;                     // 连接名称
    QString uuid;                   // 唯一标识符（新建时为空，由 NM 生成）
    QString type;                   // 如 "802-11-wireless", "802-3-ethernet"
    NetworkManager::ConnectionSettings::ConnectionType typeEnum;

    // ===== 通用行为 =====
    int timestamp = 0;
    bool autoconnect = true;
    int autoconnectPriority = 0;    // 对应 NM 的 autoconnect-priority
    bool active = false;            // 运行时激活状态（仅用于展示，不存储）

    // ===== 通用接口 =====
    QString interfaceName;          // 绑定的接口名，如 "wlan0", "eth0"

    // ===== 无线 (Wi-Fi) 专用 =====
    QString ssid;                   // SSID（802-11-wireless.ssid）
    QString wirelessSecurity;       // 如 "none", "wpa-psk", "wpa-eap"
    QString wirelessPassword;       // PSK 密码（仅用于 wpa-psk 等）
    // 若需支持 EAP，可扩展更多字段，但基本够用

    // ===== 有线专用 =====
    int mtu = 0;                    // 0 表示自动协商

    // ===== IPv4 设置 =====
    QString ipv4Method;             // "auto", "manual", "disabled" 等
    QString ipv4Address;            // 如 "192.168.1.100/24"
    QString ipv4Gateway;
    QStringList ipv4Dns;

    // // ===== IPv6 设置 (可选) =====
    // QString ipv6Method;             // "auto", "manual", "disabled"
    // QString ipv6Address;
    // QString ipv6Gateway;
    // QStringList ipv6Dns;

    // ===== 扩展字段 (兜底) =====
    QVariantMap extra;              // 用于存放尚未提取的 NM 设置项

    // ===== 序列化 / 反序列化 =====
    static ConnectionSettingInfo fromNMSettings(
        const NetworkManager::ConnectionSettings::Ptr &settings,
        bool active = false);

    QVariantMap toNMSettings() const;
};

} // namespace Net