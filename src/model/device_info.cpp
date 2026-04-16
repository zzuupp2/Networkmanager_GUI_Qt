#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/ActiveConnection>

#include "device_info.h"
#include "src/utils/network_utils.h"

DeviceInfo buildDeviceInfo(const NetworkManager::Device::Ptr &dev) {
    DeviceInfo info;

    // ===== 基本信息 =====
    info.uni = dev->uni();
    info.name = dev->interfaceName();
    info.type = NetUtils::deviceTypeToString(dev->type());
    info.hwaddr = NetUtils::getHwAddr(dev);

    // ===== 状态 ====
    info.state = NetUtils::deviceStateToString(dev->state());
    auto reason = dev->stateReason().reason();
    qDebug()<< "reason: "<< reason;
    info.stateReason = NetUtils::stateReasonToString(reason);

    // ===== IPv4 =====
    auto ip4 = dev->ipV4Config();

    if (ip4.isValid()) {

        QStringList dnsList;
        const auto add4 = ip4.addresses();
        if (!add4.isEmpty()) {
            info.ipv4 = add4.first().ip().toString();
        }

        const auto servers = ip4.nameservers();
        info.ipv4Gateway = ip4.gateway();

        for (const auto&addr : servers) {
            dnsList << addr.toString();
        }

        info.ipv4Dns = dnsList;
    }

    // ===== IPv6 =====
    auto ip6 = dev->ipV6Config();

    if (ip6.isValid()) {

        const auto add6 = ip6.addresses();
        if (!add6.isEmpty()) {
                info.ipv6 = add6.first().ip().toString();
        }
    }
    // ===== Active Connection =====
    auto active = dev->activeConnection();
    if (active) {
        info.activeConnection = active->id();
    }

    // ===== 有线设备 =====
    if (dev->type() == NetworkManager::Device::Ethernet) {
        auto wired = dev.dynamicCast<NetworkManager::WiredDevice>();
        if (wired) {
            info.carrier = wired->carrier();
            info.speed = wired->bitRate();
        }
    }

    // ===== 无线设备 =====
    if (dev->type() == NetworkManager::Device::Wifi) {
        auto wifi = dev.dynamicCast<NetworkManager::WirelessDevice>();
        if (wifi && wifi->activeAccessPoint()) {
            info.signal = wifi->activeAccessPoint()->signalStrength();
        }
    }

    return info;
}