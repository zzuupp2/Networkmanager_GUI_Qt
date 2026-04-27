#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/ActiveConnection>

#include "device_info.h"
#include "src/utils/network_utils.h"

namespace Net {

    DeviceInfo DeviceInfo::fromDevice(const NetworkManager::Device::Ptr &dev) {
        DeviceInfo info;

        if (!dev)
            return info;

        // ===== 基本信息 =====
        info.uni = dev->uni();
        info.name = dev->interfaceName();
        info.type = NetUtils::deviceTypeToString(dev->type());
        info.typeEnum = dev->type();
        info.mtu = dev->mtu();
        // info.hwaddr = NetUtils::getHwAddr(dev);

        // ===== 状态 =====
        auto state = dev->state();
        info.state = NetUtils::deviceStateToString(state);
        info.stateReason = NetUtils::stateReasonToString(dev->stateReason().reason());

        // info.ipv4.clear();
        // info.ipv4Gateway.clear();
        // info.ipv4Dns.clear();

        // auto ip4 = dev->ipV4Config();

        // if (ip4.isValid()) {

        //     const auto add4 = ip4.addresses();
        //     if (!add4.isEmpty()) {
        //         info.ipv4 = add4.first().ip().toString();
        //     }

        //     info.ipv4Gateway = ip4.gateway();

        //     QStringList dnsList;
        //     const auto servers = ip4.nameservers();
        //     for (const auto &addr : servers) {
        //         dnsList << addr.toString();
        //     }

        //     info.ipv4Dns = dnsList;
        // }

        // ===== Active Connection =====
        auto active = dev->activeConnection();
        if (active) {
            info.activeConnection = active->id();
            info.activeConUuid = active->uuid();

            // qDebug() << "actId: " << info.activeConnection;
            // qDebug() << "actUuid: " << info.activeConUuid;
        }

        // ===== 有线 or 无线设备 =====
        // if (NetUtils::isEthernetDev(dev)) {

        //     auto wired = dev.dynamicCast<NetworkManager::WiredDevice>();
        //     if (wired) {

        //         info.carrier = wired->carrier();
        //         int speed = wired->bitRate(); // kbps

        //         auto formatSpeed = [](int kbps) -> QString {
        //             if (kbps <= 0)
        //                 return QString();

        //             double mbps = kbps / 1000.0;

        //             if (mbps >= 1000)
        //                 return QString::number(mbps / 1000.0, 'f', 1) + " Gbps";
        //             return QString::number(mbps, 'f', 0) + " Mbps";
        //         };


        // } else if (NetUtils::isWifiDev(dev)) {

        //     auto wifi = dev.dynamicCast<NetworkManager::WirelessDevice>();

        //     if (wifi && wifi->activeAccessPoint()
        //         && state == NetworkManager::Device::Activated) {

        //         int signal = wifi->activeAccessPoint()->signalStrength();

        //         info.wiredOrWireless =
        //             QString("WiFi 已连接，信号: %1%").arg(signal);

        //     } else {
        //         info.wiredOrWireless = "WiFi 未连接";
        //     }
        // }

        return info;
    }
}