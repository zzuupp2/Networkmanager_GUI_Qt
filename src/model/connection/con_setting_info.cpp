#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Ipv4Setting>

#include "con_setting_info.h"
#include "src/utils/network_utils.h"

namespace Net {

ConnectionSettingInfo ConnectionSettingInfo::fromNMSettings(
    const NetworkManager::ConnectionSettings::Ptr &settings,
    bool active)
{
    ConnectionSettingInfo info;

    if (!settings)
        return info;

    using R = NetworkManager::Setting;

    // connection
    info.id = settings->id();
    info.uuid = settings->uuid();
    info.type = NetworkManager::ConnectionSettings::typeAsString(settings->connectionType());
    info.typeEnum = settings->connectionType();
    info.active = active;
    info.timestamp = settings->timestamp();

    info.autoconnect = settings->autoconnect();
    info.autoconnectPriority = settings->autoconnectPriority();
    info.interfaceName = settings->interfaceName();

    // wired
    if (NetUtils::isWiredConType(settings)) {
        auto wired = settings->setting(R::Wired)
        .staticCast<NetworkManager::WiredSetting>();

        if (wired) {
            info.mtu = wired->mtu();
            info.wiredSpeed = static_cast<int>(wired->speed());
            info.wiredDuplex = [](NetworkManager::WiredSetting::DuplexType d) -> QString {
                switch (d) {
                case NetworkManager::WiredSetting::Half: return QStringLiteral("half");
                case NetworkManager::WiredSetting::Full: return QStringLiteral("full");
                default: return QString();
                }
            }(wired->duplexType());
            // autoNegotiate: DBus bool 转三态 (-1=未设/0=手动/1=自动)
            // DBus 默认 true，无法区分"未设"与"自动"。统一用 extra 缓存原始值，
            // 若 extra 无记录则假定为自动(1)
            info.wiredAutoNegotiate = wired->autoNegotiate() ? 1 : 0;
            info.extra["mac"] = wired->macAddress();
        }
    }

    // wireless
    if (NetUtils::isWirelessConType(settings)) {

        auto wifi = settings->setting(R::Wireless)
        .staticCast<NetworkManager::WirelessSetting>();

        if (wifi) {
            info.ssid = wifi->ssid();
            info.extra["mode"] = wifi->mode();
        }

        auto sec = settings->setting(R::WirelessSecurity)
                       .staticCast<NetworkManager::WirelessSecuritySetting>();

        if (sec) {
            info.wirelessSecurity = NetUtils::keyMgmtToString(sec->keyMgmt());
            info.wirelessPassword = sec->psk();

            if (!sec->leapUsername().isEmpty())
                info.extra["leap-username"] = sec->leapUsername();
        } else {
            info.wirelessSecurity = "none";
        }
    }

    // ipv4
    auto ipv4 = settings->setting(R::Ipv4)
                    .staticCast<NetworkManager::Ipv4Setting>();

    if (ipv4) {
        info.ipv4Method = NetUtils::ipv4MethodToString(ipv4->method());

        if (info.ipv4Method == "manual") {

            const auto addrs = ipv4->addresses();

            if (!addrs.isEmpty()) {
                const auto &a = addrs.first();
                info.ipv4Address =
                    a.ip().toString() + "/" +
                    QString::number(a.prefixLength());
                info.ipv4Gateway = a.gateway().toString();
            }

            QStringList dns;
            for (const auto &d : ipv4->dns())
                dns << d.toString();

            info.ipv4Dns = dns;
        }
    } else {
        info.ipv4Method = "auto";
    }

    // extra merge (safe)
    NMVariantMapMap all = settings->toMap();

    QStringList mappedKeys = {
        "connection",
        "802-3-ethernet",
        "802-11-wireless",
        "802-11-wireless-security",
        "ipv4",
        "ipv6"
    };

    QVariantMap mergedExtra = info.extra;

    for (auto it = all.begin(); it != all.end(); ++it) {
        if (!mappedKeys.contains(it.key()))
            mergedExtra.insert(it.key(), it.value());
    }

    info.extra = mergedExtra;

    return info;
}

// struct -> NM

NMVariantMapMap ConnectionSettingInfo::toNMSettings() const
{
    NMVariantMapMap map;

    // connection
    QVariantMap conn;

    conn["id"] = id;
    conn["type"] = type;
    conn["autoconnect"] = autoconnect;
    conn["autoconnect-priority"] = autoconnectPriority;
    conn["interface-name"] = interfaceName;

    if (!uuid.isEmpty())
        conn["uuid"] = uuid;

    map.insert("connection", conn);

    // ethernet
    if (type == "802-3-ethernet") {

        QVariantMap wired;

        if (mtu > 0)
            wired["mtu"] = mtu;

        if (wiredAutoNegotiate == 0) {
            // 手动模式：禁用协商，写 speed + duplex
            wired["auto-negotiate"] = false;
            if (wiredSpeed > 0)
                wired["speed"] = wiredSpeed;
            if (!wiredDuplex.isEmpty())
                wired["duplex"] = wiredDuplex;
        } else if (wiredAutoNegotiate == 1) {
            // 自动模式：启用协商
            wired["auto-negotiate"] = true;
        }
        // wiredAutoNegotiate == -1（忽略）：不写任何值

        if (extra.contains("mac"))
            wired["mac-address"] = extra["mac"];

        if (!wired.isEmpty())
            map.insert("802-3-ethernet", wired);
    }

    // wifi
    if (type == "802-11-wireless") {

        QVariantMap wifi;

        if (!ssid.isEmpty())
            wifi["ssid"] = ssid.toUtf8();   // ✔ 修复点

        if (extra.contains("mode"))
            wifi["mode"] = extra["mode"];

        if (!wifi.isEmpty())
            map.insert("802-11-wireless", wifi);

        QVariantMap sec;

        if (!wirelessSecurity.isEmpty() && wirelessSecurity != "none") {

            sec["key-mgmt"] = wirelessSecurity;

            if ((wirelessSecurity == "wpa-psk" ||
                 wirelessSecurity == "wpa-psk-sha256") &&
                !wirelessPassword.isEmpty()) {
                sec["psk"] = wirelessPassword;
            }

            if (wirelessSecurity == "wpa-eap") {
                if (extra.contains("leap-username"))
                    sec["leap-username"] = extra["leap-username"];
            }
        }

        if (!sec.isEmpty())
            map.insert("802-11-wireless-security", sec);
    }

    // ipv4
    QVariantMap ipv4;

    const QString method =
        (ipv4Method == "auto" ||
         ipv4Method == "manual" ||
         ipv4Method == "disabled" ||
         ipv4Method == "shared" ||
         ipv4Method == "link-local")
            ? ipv4Method
            : "auto";

    ipv4["method"] = method;

    if (method == "manual") {

        QVariantMap addr;

        QString ip = ipv4Address;
        QString prefix = "24";

        if (ipv4Address.contains('/')) {
            auto parts = ipv4Address.split('/');
            ip = parts.value(0);
            prefix = parts.value(1);
        }

        addr["address"] = ip;
        addr["prefix"] = prefix.toInt();

        if (!ipv4Gateway.isEmpty())
            addr["gateway"] = ipv4Gateway;

        QVariantList addrs;
        addrs << addr;

        ipv4["addresses"] = addrs;

        QVariantList dns;
        for (const auto &d : ipv4Dns)
            dns << d;

        if (!dns.isEmpty())
            ipv4["dns"] = dns;
    }

    map.insert("ipv4", ipv4);

    return map;
}

} // namespace Net