#include "network_utils.h"

namespace NetUtils {

    QString deviceTypeToString(NetworkManager::Device::Type type) {
        switch (type) {
            using R = NetworkManager::Device::Type;
            case R::UnknownType: return "Unknown";

            case R::Ethernet: return "Ethernet";
            case R::Wifi: return "WiFi";

            case R::Unused1: return "Unused1";
            case R::Unused2: return "Unused2";

            case R::Bluetooth: return "Bluetooth";
            case R::OlpcMesh: return "OLPC Mesh";
            case R::Wimax: return "WiMAX";
            case R::Modem: return "Modem";
            case R::InfiniBand: return "InfiniBand";

            case R::Bond: return "Bond";
            case R::Vlan: return "VLAN";
            case R::Adsl: return "ADSL";
            case R::Bridge: return "Bridge";
            case R::Generic: return "Generic";
            case R::Team: return "Team";

            case R::Gre: return "GRE";
            case R::MacVlan: return "MacVLAN";
            case R::Tun: return "TUN";
            case R::Veth: return "Veth";
            case R::IpTunnel: return "IP Tunnel";
            case R::VxLan: return "VXLAN";
            case R::MacSec: return "MACsec";
            case R::Dummy: return "Dummy";
            case R::Ppp: return "PPP";

            case R::OvsInterface: return "OVS Interface";
            case R::OvsPort: return "OVS Port";
            case R::OvsBridge: return "OVS Bridge";

            case R::Wpan: return "WPAN";
            case R::Lowpan: return "LoWPAN";

            case R::WireGuard: return "WireGuard";
            case R::WifiP2P: return "WiFi P2P";
            case R::VRF: return "VRF";
            case R::Loopback: return "Loopback";

            default:
                return "Unknown";
        }
    }

    QString deviceStateToString(NetworkManager::Device::State state) {
        using R = NetworkManager::Device::State;
        switch (state) {
        case R::UnknownState: return "Unknown";
        case R::Unmanaged: return "Unmanaged";
        case R::Unavailable: return "Unavailable";
        case R::Disconnected: return "Disconnected";

        case R::Preparing: return "Preparing";
        case R::ConfiguringHardware: return "Configuring Hardware";
        case R::NeedAuth: return "Authentication Required";
        case R::ConfiguringIp: return "Configuring IP";
        case R::CheckingIp: return "Checking IP";
        case R::WaitingForSecondaries: return "Waiting for Secondary Connections";

        case R::Activated: return "Activated";
        case R::Deactivating: return "Disconnecting";
        case R::Failed: return "Failed";

        default:
            return "Unknown";
        }
    }

    QString stateReasonToString(NetworkManager::Device::StateChangeReason reason)
    {
        using R = NetworkManager::Device::StateChangeReason;

        switch (reason) {
        case R::UnknownReason: return "Unknown";
        case R::NoReason: return "No reason";
        case R::NowManagedReason: return "Device is now managed";
        case R::NowUnmanagedReason: return "Device is now unmanaged";

        case R::ConfigFailedReason: return "Configuration failed";
        case R::ConfigUnavailableReason: return "Configuration unavailable";
        case R::ConfigExpiredReason: return "Configuration expired";

        case R::NoSecretsReason: return "Missing authentication information";

        case R::AuthSupplicantDisconnectReason: return "Authentication disconnected";
        case R::AuthSupplicantConfigFailedReason: return "Authentication config failed";
        case R::AuthSupplicantFailedReason: return "Authentication failed";
        case R::AuthSupplicantTimeoutReason: return "Authentication timeout";

        case R::DhcpStartFailedReason: return "DHCP start failed";
        case R::DhcpErrorReason: return "DHCP error";
        case R::DhcpFailedReason: return "DHCP failed";

        case R::AutoIpStartFailedReason: return "Auto IP start failed";
        case R::AutoIpErrorReason: return "Auto IP error";
        case R::AutoIpFailedReason: return "Auto IP failed";

        case R::UserRequestedReason: return "Disconnected by user";
        case R::CarrierReason: return "Carrier/link changed";

        case R::ConnectionRemovedReason: return "Connection removed";
        case R::ConnectionAssumedReason: return "Connection assumed";

        case R::SsidNotFound: return "WiFi network not found";

        case R::FirmwareMissingReason: return "Firmware missing";
        case R::DeviceRemovedReason: return "Device removed";

        case R::ModemNotFoundReason: return "Modem not found";
        case R::ModemBusyReason: return "Modem busy";
        case R::ModemFailed: return "Modem failed";

        case R::GsmSimNotInserted: return "SIM not inserted";
        case R::GsmSimPinRequired: return "SIM PIN required";
        case R::GsmSimPukRequired: return "SIM PUK required";
        case R::GsmSimWrong: return "SIM error";
        case R::SimPinIncorrect: return "SIM PIN incorrect";

        case R::DependencyFailed: return "Dependency failed";

        case R::NewActivation: return "New activation started";

        default:
            return QString("Unknown reason (%1)").arg(static_cast<int>(reason));
        }
    }

    QString getHwAddr(const NetworkManager::Device::Ptr &dev) {
        QString result;
        if (dev) {

            auto wired = dev.objectCast<NetworkManager::WiredDevice>();
            if (wired) {

                result = wired->hardwareAddress();
            } else {

                auto wifi = dev.objectCast<NetworkManager::WirelessDevice>();
                if (wifi) {

                    result = wifi->hardwareAddress();
                }
            }
        }

        return result;
    }

    bool isDeviceType(const NetworkManager::Device::Ptr &dev,
                      NetworkManager::Device::Type devType) {

        if (!dev)
            return false;

        return dev->type() == devType;
    }

    bool isNetworkDevice(const NetworkManager::Device::Ptr &dev) {
        using R = NetworkManager::Device;
        if (!dev)
            return false;

        auto type = dev->type();

        return type == R::Wifi ||
               type == R::Ethernet;
    }

    bool isWifiDev(const NetworkManager::Device::Ptr &dev) {
        using R = NetworkManager::Device;
        return isDeviceType(dev,
                           R::Wifi);
    }

    bool isEthernetDev(const NetworkManager::Device::Ptr &dev) {
        using R = NetworkManager::Device;
        return isDeviceType(dev,
                            R::Ethernet);
    }

    bool isConSetType(const NetworkManager::ConnectionSettings::Ptr &set,
                      NetworkManager::ConnectionSettings::ConnectionType conType) {

        if (!set)
            return false;

        return set->connectionType() == conType;
    }

    bool isWiredConType(const NetworkManager::ConnectionSettings::Ptr &set) {
        using S = NetworkManager::ConnectionSettings::ConnectionType;

        return isConSetType(set,
                            S::Wired);
    }


    bool isWirelessConType(const NetworkManager::ConnectionSettings::Ptr &set) {
        using S = NetworkManager::ConnectionSettings::ConnectionType;
        return isConSetType(set,
                            S::Wireless);
    }

    // bool isConActive(const QString &uuid) {
    //     for (const auto &ac : )
    // }
}