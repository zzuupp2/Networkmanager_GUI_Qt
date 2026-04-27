#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>

#include "con_setting_info.h"
#include "src/utils/network_utils.h"

namespace Net {

ConnectionSettingInfo ConnectionSettingInfo::fromNMSettings(
    const NetworkManager::ConnectionSettings::Ptr &settings,
    bool active)
{
    ConnectionSettingInfo info;
    using R = NetworkManager::Setting;

    if (!settings)
        return info;

    // ===== 通用标识 =====
    info.id = settings->id();
    info.uuid = settings->uuid();
    info.type = NetworkManager::ConnectionSettings::typeAsString(settings->connectionType());
    info.typeEnum = settings->connectionType();

    // ===== 通用设置 =====
    info.autoconnect = settings->autoconnect();
    info.autoconnectPriority = settings->autoconnectPriority();
    info.interfaceName = settings->interfaceName();

    // ===== 有线设置 =====
    if (NetUtils::isWiredConType(settings)) {
        auto wired = settings->setting(R::Wired)
        .staticCast<NetworkManager::WiredSetting>();
        if (wired) {
            info.mtu = wired->mtu();
            // 其他未直接映射的有线属性存入 extra
            info.extra["mac"] = wired->macAddress();
            info.extra["speed"] = wired->speed();
            info.extra["auto_negotiate"] = wired->autoNegotiate();
        }
    }

    // ===== 无线设置 =====
    if (NetUtils::isWirelessConType(settings)) {
        auto wifi = settings->setting(R::Wireless)
        .staticCast<NetworkManager::WirelessSetting>();
        if (wifi) {
            info.ssid = wifi->ssid();
            info.extra["mode"] = wifi->mode();   // Ad-Hoc 等模式暂时留在 extra
        }

        // 安全设置
        auto sec = settings->setting(R::WirelessSecurity)
                       .staticCast<NetworkManager::WirelessSecuritySetting>();
        if (sec) {
            // 将 NM 的安全类型转换为简化的字符串
            auto keyMgmt = sec->keyMgmt();
            QString keyMgmtStr;
            if (keyMgmt == NetworkManager::WirelessSecuritySetting::Unknown)
                keyMgmtStr = "unknown";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Wep)
                keyMgmtStr = "wep";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x)
                keyMgmtStr = "iee8021x";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaNone)
                keyMgmtStr = "wpa-none";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaPsk)
                keyMgmtStr = "wpa-psk";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEap)
                keyMgmtStr = "wpa-eap";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::SAE)
                keyMgmtStr = "sae";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192)
                keyMgmtStr = "wpa-eap-suiteb192";
            else if (keyMgmt == NetworkManager::WirelessSecuritySetting::OWE)
                keyMgmtStr = "owe";

            info.wirelessSecurity = keyMgmtStr;
            // 密码（仅 PSK 等需要）
            info.wirelessPassword = sec->psk();
            // 其他安全细节可放入 extra
            if (!sec->leapUsername().isEmpty())
                info.extra["leap-username"] = sec->leapUsername();
        } else {
            info.wirelessSecurity = "none";
        }
    }

    // ===== IPv4 设置 =====
    auto ipv4 = settings->setting(R::Ipv4)
                    .staticCast<NetworkManager::Ipv4Setting>();
    if (ipv4) {

        auto ip4Method = ipv4->method();
        switch (ip4Method)  {
        case NetworkManager::Ipv4Setting::ConfigMethod::Automatic
            : info.ipv4Method = "auto"; break;
        case NetworkManager::Ipv4Setting::ConfigMethod::Disabled
            : info.ipv4Method = "disable"; break;
        case NetworkManager::Ipv4Setting::ConfigMethod::LinkLocal
            : info.ipv4Method = "link-loacl"; break;
        case NetworkManager::Ipv4Setting::ConfigMethod::Manual
            : info.ipv4Method = "manual"; break;
        case NetworkManager::Ipv4Setting::ConfigMethod::Shared
            : info.ipv4Method = "shared"; break;
        default:
            break;
        }

        if (info.ipv4Method == "manual") {

            // 仅取第一个地址和网关
            if (!ipv4->addresses().isEmpty()) {
                const auto &addr = ipv4->addresses().first();
                info.ipv4Address = addr.ip().toString() + "/" + QString::number(addr.prefixLength());
                info.ipv4Gateway = addr.gateway().toString();
            }

            // DNS
            QStringList dns;

            for (const auto &a : ipv4->dns()) {
                dns << a.toString();
            }

            info.ipv4Dns = dns;
        }

    } else {
        info.ipv4Method = "auto";   // 默认
    }

    // ===== IPv6 设置 =====
    // auto ipv6 = settings->setting(R::Ipv6)
    //                 .staticCast<NetworkManager::Ipv6Setting>();
    // if (ipv6) {
    //     info.ipv6Method = ipv6->method();
    //     if (ipv6->method() == "manual") {
    //         if (!ipv6->addresses().isEmpty()) {
    //             const auto &addr = ipv6->addresses().first();
    //             info.ipv6Address = addr.ip().toString() + "/" + QString::number(addr.prefixLength());
    //             info.ipv6Gateway = addr.gateway().toString();
    //         }
    //         QStringList dns;
    //         for (const auto &a : ipv6->dns()) {
    //             dns << a.toString();
    //         }
    //         info.ipv6Dns = dns;
    //     }
    // } else {
    //     info.ipv6Method = "auto";
    // }

    // 保存所有未被显式提取的其余设置
    NMVariantMapMap allSettings = settings->toMap();
    // 移除已映射的顶层键，避免重复
    QStringList mappedKeys = {"connection", "802-3-ethernet", "802-11-wireless",
                              "802-11-wireless-security", "ipv4", "ipv6"};
    for (const auto &key : mappedKeys)
        allSettings.remove(key);
    // 合并已存入 extra 的内容（手动添加的有线/无线扩展）
    QVariantMap mergedExtra = info.extra;
    for (auto it = allSettings.cbegin(); it != allSettings.cend(); ++it) {
        mergedExtra.insert(it.key(), it.value());   // 其他未识别的设置
    }
    info.extra = mergedExtra;

    return info;
}

QVariantMap ConnectionSettingInfo::toNMSettings() const
{
    QVariantMap map;

    // 连接通用设置
    QVariantMap connectionMap;
    connectionMap["id"] = id;
    connectionMap["type"] = type;
    connectionMap["autoconnect"] = autoconnect;
    connectionMap["autoconnect-priority"] = autoconnectPriority;
    connectionMap["interface-name"] = interfaceName;
    if (!uuid.isEmpty())
        connectionMap["uuid"] = uuid;
    map["connection"] = connectionMap;

    // 有线设置
    if (type == "802-3-ethernet") {
        QVariantMap wiredMap;
        if (mtu != 0)
            wiredMap["mtu"] = mtu;
        // 从 extra 恢复有线专有字段（如果存在）
        if (extra.contains("mac"))
            wiredMap["mac-address"] = extra["mac"];
        if (extra.contains("speed"))
            wiredMap["speed"] = extra["speed"];
        if (extra.contains("auto_negotiate"))
            wiredMap["auto-negotiate"] = extra["auto_negotiate"];
        if (!wiredMap.isEmpty())
            map["802-3-ethernet"] = wiredMap;
    }

    // 无线设置
    if (type == "802-11-wireless") {
        QVariantMap wirelessMap;
        if (!ssid.isEmpty())
            wirelessMap["ssid"] = ssid;
        if (extra.contains("mode"))
            wirelessMap["mode"] = extra["mode"];
        if (!wirelessMap.isEmpty())
            map["802-11-wireless"] = wirelessMap;

        // 无线安全
        QVariantMap secMap;
        if (wirelessSecurity == "none" || wirelessSecurity.isEmpty()) {
            // 不写入安全设置或设置为 none
        } else {
            secMap["key-mgmt"] = wirelessSecurity;
            if (wirelessSecurity == "wpa-psk" || wirelessSecurity == "wpa-psk-sha256") {
                if (!wirelessPassword.isEmpty())
                    secMap["psk"] = wirelessPassword;
            } else if (wirelessSecurity == "wpa-eap") {
                // 处理 EAP 具体字段（从 extra 恢复，如 leap-username）
                if (extra.contains("leap-username"))
                    secMap["leap-username"] = extra["leap-username"];
            }
        }
        if (!secMap.isEmpty())
            map["802-11-wireless-security"] = secMap;
    }

    // IPv4 设置
    QVariantMap ipv4Map;
    if (!ipv4Method.isEmpty() && ipv4Method != "auto") {
        ipv4Map["method"] = ipv4Method;
        if (ipv4Method == "manual") {
            // 构造地址数组
            QVariantList addresses;
            QVariantMap addrMap;
            // 解析 ipv4Address
            QString ip, mask;
            if (ipv4Address.contains('/')) {
                QStringList parts = ipv4Address.split('/');
                ip = parts[0];
                mask = parts[1];
            } else {
                ip = ipv4Address;
                mask = "24";
            }
            addrMap["address"] = ip;
            addrMap["prefix"] = mask.toInt();
            if (!ipv4Gateway.isEmpty())
                addrMap["gateway"] = ipv4Gateway;
            addresses.append(addrMap);
            ipv4Map["addresses"] = addresses;
            // DNS
            QVariantList dnsList;
            for (const auto &d : ipv4Dns)
                dnsList << d;
            if (!dnsList.isEmpty())
                ipv4Map["dns"] = dnsList;
        }
    } else {
        ipv4Map["method"] = "auto";
    }
    map["ipv4"] = ipv4Map;

    // // IPv6 设置
    // QVariantMap ipv6Map;
    // if (!ipv6Method.isEmpty() && ipv6Method != "auto") {
    //     ipv6Map["method"] = ipv6Method;
    //     if (ipv6Method == "manual") {
    //         // 类似 IPv4 的处理
    //         QVariantList addresses;
    //         QVariantMap addrMap;
    //         QString ip, mask;
    //         if (ipv6Address.contains('/')) {
    //             QStringList parts = ipv6Address.split('/');
    //             ip = parts[0];
    //             mask = parts[1];
    //         } else {
    //             ip = ipv6Address;
    //             mask = "64";
    //         }
    //         addrMap["address"] = ip;
    //         addrMap["prefix"] = mask.toInt();
    //         if (!ipv6Gateway.isEmpty())
    //             addrMap["gateway"] = ipv6Gateway;
    //         addresses.append(addrMap);
    //         ipv6Map["addresses"] = addresses;
    //         QVariantList dnsList;
    //         for (const auto &d : ipv6Dns)
    //             dnsList << d;
    //         if (!dnsList.isEmpty())
    //             ipv6Map["dns"] = dnsList;
    //     }
    // } else {
    //     ipv6Map["method"] = "auto";
    // }
    // map["ipv6"] = ipv6Map;

    // 合并 extra 中未被识别的设置（确保不丢失自定义属性）
    for (auto it = extra.constBegin(); it != extra.constEnd(); ++it) {
        // 跳过已明确处理过的键，避免覆盖
        if (it.key() == "mac" || it.key() == "speed" || it.key() == "auto_negotiate" ||
            it.key() == "mode" || it.key() == "leap-username")
            continue;
        // 简单地将额外键值对直接附加到顶层（或者根据需要放入正确的子映射）
        // 这里采用直接附加到 map，可被 NetworkManager 忽略或处理
        map[it.key()] = it.value();
    }

    return map;
}

} // namespace Net