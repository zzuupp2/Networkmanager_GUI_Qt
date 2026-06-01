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

        info.uni = dev->uni();
        info.name = dev->interfaceName();
        info.type = NetUtils::deviceTypeToString(dev->type());
        info.typeEnum = dev->type();
        info.mtu = dev->mtu();
        info.hwaddr = NetUtils::getHwAddr(dev);

        info.driver = dev->driver();
        info.driverVersion = dev->driverVersion();
        info.firmwareVersion = dev->firmwareVersion();


        auto state = dev->state();
        info.stateEnum = state;
        auto reason = dev->stateReason().reason();
        info.state = NetUtils::deviceStateToString(state);
        info.stateSummary = NetUtils::stateReasonSummary(state, reason);
        info.stateReason = NetUtils::stateReasonToString(reason);

        auto active = dev->activeConnection();
        if (active) {
            info.activeConnection = active->id();
            info.activeConUuid = active->uuid();
        }

        if (auto wired = dev.objectCast<NetworkManager::WiredDevice>()) {
            info.bitRate = wired->bitRate();
            info.carrier = wired->carrier();
        }

        if (auto wifi = dev.objectCast<NetworkManager::WirelessDevice>()) {
            auto ap = wifi->activeAccessPoint();
            if (ap && state == NetworkManager::Device::Activated) {
                info.activeSsid = ap->ssid();
                info.signalStrength = ap->signalStrength();
            }
        }

        return info;
    }
}
