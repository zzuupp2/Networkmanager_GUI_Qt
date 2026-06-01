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

    QString stateReasonSummary(NetworkManager::Device::State state,
                               NetworkManager::Device::StateChangeReason reason)
    {
        using S = NetworkManager::Device::State;
        using R = NetworkManager::Device::StateChangeReason;

        // Activated
        if (state == S::Activated)
            return QStringLiteral("已连接");

        // Disconnected
        if (state == S::Disconnected) {
            switch (reason) {
            case R::UserRequestedReason:   return QStringLiteral("用户断开连接");
            case R::CarrierReason:         return QStringLiteral("未连接");
            case R::ConnectionRemovedReason: return QStringLiteral("连接配置已删除");
            case R::SsidNotFound:          return QStringLiteral("Wi-Fi 网络不在范围内");
            case R::SleepingReason:        return QStringLiteral("设备休眠");
            case R::ConnectionAssumedReason: return QStringLiteral("连接已被其他程序接管");
            case R::DeviceRemovedReason:   return QStringLiteral("设备已移除");
            case R::SupplicantAvailableReason: return QStringLiteral("认证服务已恢复，等待重新连接");
            case R::DependencyFailed:      return QStringLiteral("依赖的另一连接失败");
            case R::NewActivation:         return QStringLiteral("正在切换到新的连接");
            case R::ParentChanged:         return QStringLiteral("父设备已变更");
            case R::ParentManagedChanged:  return QStringLiteral("父设备管理状态变更");
            case R::NoReason:              return QStringLiteral("未连接");
            default:                       return QStringLiteral("未连接");
            }
        }

        // Failed
        if (state == S::Failed) {
            switch (reason) {
            case R::ConfigFailedReason:    return QStringLiteral("连接失败 — 配置错误");
            case R::ConfigUnavailableReason: return QStringLiteral("连接失败 — 配置不可用");
            case R::ConfigExpiredReason:   return QStringLiteral("连接失败 — 配置已过期");
            case R::NoSecretsReason:       return QStringLiteral("连接失败 — 缺少密码或密钥");
            case R::AuthSupplicantFailedReason: return QStringLiteral("连接失败 — 身份验证失败");
            case R::AuthSupplicantTimeoutReason: return QStringLiteral("连接失败 — 身份验证超时");
            case R::AuthSupplicantDisconnectReason: return QStringLiteral("连接失败 — 认证服务断开");
            case R::AuthSupplicantConfigFailedReason: return QStringLiteral("连接失败 — 认证配置错误");
            case R::DhcpStartFailedReason: return QStringLiteral("连接失败 — DHCP 启动失败");
            case R::DhcpErrorReason:       return QStringLiteral("连接失败 — DHCP 错误");
            case R::DhcpFailedReason:      return QStringLiteral("连接失败 — DHCP 获取 IP 失败");
            case R::AutoIpStartFailedReason: return QStringLiteral("连接失败 — 自动 IP 启动失败");
            case R::AutoIpErrorReason:     return QStringLiteral("连接失败 — 自动 IP 错误");
            case R::AutoIpFailedReason:    return QStringLiteral("连接失败 — 自动 IP 分配失败");
            case R::PppStartFailedReason:  return QStringLiteral("连接失败 — PPP 启动失败");
            case R::PppFailedReason:       return QStringLiteral("连接失败 — PPP 拨号失败");
            case R::PppDisconnectReason:   return QStringLiteral("连接失败 — PPP 连接断开");
            case R::SharedStartFailedReason: return QStringLiteral("连接失败 — 共享连接启动失败");
            case R::SharedFailedReason:    return QStringLiteral("连接失败 — 共享连接失败");
            case R::ModemNotFoundReason:   return QStringLiteral("连接失败 — 调制解调器未找到");
            case R::ModemBusyReason:       return QStringLiteral("连接失败 — 调制解调器忙");
            case R::ModemFailed:           return QStringLiteral("连接失败 — 调制解调器故障");
            case R::ModemAvailable:        return QStringLiteral("连接失败 — 调制解调器状态异常");
            case R::GsmSimNotInserted:     return QStringLiteral("连接失败 — 未插入 SIM 卡");
            case R::GsmSimPinRequired:     return QStringLiteral("连接失败 — 需要 SIM PIN 码");
            case R::GsmSimPukRequired:     return QStringLiteral("连接失败 — 需要 SIM PUK 码");
            case R::GsmSimWrong:           return QStringLiteral("连接失败 — SIM 卡错误");
            case R::SimPinIncorrect:       return QStringLiteral("连接失败 — SIM PIN 码错误");
            case R::DependencyFailed:      return QStringLiteral("连接失败 — 依赖连接失败");
            case R::DeviceRemovedReason:   return QStringLiteral("连接失败 — 设备已移除");
            case R::FirmwareMissingReason: return QStringLiteral("连接失败 — 缺少固件");
            case R::SsidNotFound:          return QStringLiteral("连接失败 — Wi-Fi 网络未找到");
            case R::SecondaryConnectionFailed: return QStringLiteral("连接失败 — 次要连接失败");
            case R::NoReason:              return QStringLiteral("连接失败");
            default:                       return QStringLiteral("连接失败");
            }
        }

        // Unmanaged
        if (state == S::Unmanaged) {
            switch (reason) {
            case R::NowManagedReason:      return QStringLiteral("已被 NetworkManager 接管");
            case R::ConnectionRemovedReason: return QStringLiteral("连接已移除，设备脱离管理");
            case R::DeviceRemovedReason:   return QStringLiteral("设备已移除");
            default:                       return QStringLiteral("未由 NetworkManager 管理");
            }
        }

        // Unavailable
        if (state == S::Unavailable) {
            switch (reason) {
            case R::FirmwareMissingReason: return QStringLiteral("设备固件缺失，无法使用");
            case R::DeviceRemovedReason:   return QStringLiteral("设备已移除");
            case R::CarrierReason:         return QStringLiteral("链路暂不可用");
            case R::ModemNotFoundReason:   return QStringLiteral("调制解调器未找到");
            case R::GsmSimNotInserted:     return QStringLiteral("SIM 卡未插入");
            case R::GsmSimPinRequired:     return QStringLiteral("需要 SIM PIN 码");
            case R::GsmSimPukRequired:     return QStringLiteral("需要 SIM PUK 码");
            case R::SleepingReason:        return QStringLiteral("设备休眠中");
            case R::NowUnmanagedReason:    return QStringLiteral("设备已脱离管理");
            case R::ParentManagedChanged:  return QStringLiteral("父设备管理状态变更");
            case R::DependencyFailed:      return QStringLiteral("依赖设备不可用");
            default:                       return QStringLiteral("设备不可用");
            }
        }

        // Connecting phases
        if (state >= S::Preparing && state <= S::WaitingForSecondaries) {
            switch (reason) {
            case R::NewActivation:         return QStringLiteral("正在激活连接");
            case R::NoReason: {
                switch (state) {
                case S::Preparing:             return QStringLiteral("正在准备连接");
                case S::ConfiguringHardware:    return QStringLiteral("正在配置硬件");
                case S::NeedAuth:              return QStringLiteral("正在进行身份验证");
                case S::ConfiguringIp:         return QStringLiteral("正在配置 IP 地址");
                case S::CheckingIp:            return QStringLiteral("正在检查 IP 连接");
                case S::WaitingForSecondaries: return QStringLiteral("正在等待辅助连接");
                default:                        return QStringLiteral("正在连接");
                }
            }
            case R::UserRequestedReason:   return QStringLiteral("用户取消连接");
            case R::DeviceRemovedReason:   return QStringLiteral("设备移除，连接中断");
            case R::CarrierReason:         return QStringLiteral("链路中断，连接中止");
            case R::DependencyFailed:      return QStringLiteral("依赖连接失败，连接中止");
            default: {
                switch (state) {
                case S::NeedAuth:              return QStringLiteral("正在验证身份");
                case S::ConfiguringIp:         return QStringLiteral("正在获取 IP 地址");
                default:                        return QStringLiteral("正在连接");
                }
            }
            }
        }

        // Deactivating
        if (state == S::Deactivating) {
            switch (reason) {
            case R::UserRequestedReason:   return QStringLiteral("用户正在断开连接");
            case R::CarrierReason:         return QStringLiteral("链路已断开，正在清理连接");
            case R::DeviceRemovedReason:   return QStringLiteral("设备移除，正在断开");
            case R::SleepingReason:        return QStringLiteral("设备休眠，正在断开连接");
            case R::ConnectionRemovedReason: return QStringLiteral("连接已删除，正在断开");
            case R::NewActivation:         return QStringLiteral("正在切换到新连接");
            default:                       return QStringLiteral("正在断开");
            }
        }

        // Unknown
        if (state == S::UnknownState) {
            switch (reason) {
            case R::FirmwareMissingReason: return QStringLiteral("状态未知 — 固件缺失");
            case R::DeviceRemovedReason:   return QStringLiteral("状态未知 — 设备已移除");
            case R::ModemNotFoundReason:   return QStringLiteral("状态未知 — 调制解调器未找到");
            default:                       return QStringLiteral("状态未知");
            }
        }

        return QStringLiteral("状态未知");
    }

    QString stateReasonSummary(NetworkManager::Device::State oldState,
                               NetworkManager::Device::State newState,
                               NetworkManager::Device::StateChangeReason reason)
    {
        using S = NetworkManager::Device::State;
        using R = NetworkManager::Device::StateChangeReason;

        // 断开后经 Unavailable 回落到 Disconnected：正常收尾，非链路丢失
        if (oldState == S::Unavailable && newState == S::Disconnected
            && reason == R::CarrierReason)
            return QStringLiteral("未连接");

        // 活跃状态下直接因 Carrier 断开：真正的链路丢失
        if ((oldState == S::Activated || oldState == S::Deactivating)
            && newState == S::Disconnected
            && reason == R::CarrierReason)
            return QStringLiteral("链路断开");

        // 活跃状态下发出断开请求
        if (newState == S::Deactivating && reason == R::UserRequestedReason)
            return QStringLiteral("用户正在断开连接");

        // 完成用户断开
        if (newState == S::Disconnected && reason == R::UserRequestedReason)
            return QStringLiteral("用户断开连接");

        // 其他情况回退到无状态版本
        return stateReasonSummary(newState, reason);
    }

    bool isMeaningfulReason(NetworkManager::Device::StateChangeReason reason)
    {
        using R = NetworkManager::Device::StateChangeReason;
        switch (reason) {
        case R::NoReason:
        case R::UnknownReason:
            return false;
        default:
            return true;
        }
    }

    bool isTransientState(NetworkManager::Device::State state)
    {
        using S = NetworkManager::Device::State;
        switch (state) {
        case S::Unavailable:
        case S::UnknownState:
            return true;
        default:
            return false;
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


    QString activeConnectionStateToString(int state)
    {
        using S = NetworkManager::ActiveConnection::State;
        switch (static_cast<S>(state)) {
        case S::Unknown:      return QStringLiteral("未知");
        case S::Activating:   return QStringLiteral("正在激活");
        case S::Activated:    return QStringLiteral("已连接");
        case S::Deactivating: return QStringLiteral("正在断开");
        case S::Deactivated:  return QStringLiteral("未连接");
        default:              return QStringLiteral("未知");
        }
    }

    QString activeConnectionReasonToString(int reason)
    {
        using R = NetworkManager::ActiveConnection::Reason;
        switch (static_cast<R>(reason)) {
        case R::UknownReason:          return QStringLiteral("未知原因");
        case R::None:                  return QStringLiteral("无");
        case R::UserDisconnected:      return QStringLiteral("用户断开连接");
        case R::DeviceDisconnected:    return QStringLiteral("设备断开连接");
        case R::ServiceStopped:        return QStringLiteral("VPN 服务已停止");
        case R::IpConfigInvalid:       return QStringLiteral("IP 配置无效");
        case R::ConnectTimeout:        return QStringLiteral("连接超时");
        case R::ServiceStartTimeout:   return QStringLiteral("服务启动超时");
        case R::ServiceStartFailed:    return QStringLiteral("服务启动失败");
        case R::NoSecrets:             return QStringLiteral("缺少密钥");
        case R::LoginFailed:           return QStringLiteral("登录失败");
        case R::ConnectionRemoved:     return QStringLiteral("连接已删除");
        case R::DependencyFailed:      return QStringLiteral("依赖连接失败");
        case R::DeviceRealizeFailed:   return QStringLiteral("设备创建失败");
        case R::DeviceRemoved:         return QStringLiteral("设备已移除");
        default:                       return QStringLiteral("未知原因");
        }
    }

    QString activeConnectionStatusText(int activeState, int stateReason)
    {
        using R = NetworkManager::ActiveConnection::Reason;
        auto reason = static_cast<R>(stateReason);

        if (reason >= R::UserDisconnected)
            return activeConnectionReasonToString(stateReason);

        return activeConnectionStateToString(activeState);
    }

    QString keyMgmtToString(NetworkManager::WirelessSecuritySetting::KeyMgmt k)
    {
        using KeyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt;

        switch (k) {
        case KeyMgmt::Wep:
            return "wep";

        case KeyMgmt::Ieee8021x:
            return "ieee8021x";

        case KeyMgmt::WpaNone:
            return "wpa-none";

        case KeyMgmt::WpaPsk:
            return "wpa-psk";

        case KeyMgmt::WpaEap:
            return "wpa-eap";

        case KeyMgmt::SAE:
            return "sae";

        case KeyMgmt::WpaEapSuiteB192:
            return "wpa-eap-suiteb-192";

        case KeyMgmt::OWE:
            return "owe";

        case KeyMgmt::Unknown:
        default:
            return "unknown";
        }
    }

    NetworkManager::WirelessSecuritySetting::KeyMgmt stringToKeyMgmt(const QString &s)
    {
        using KeyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt;

        const QString v = s.toLower();

        if (v == "wep")
            return KeyMgmt::Wep;

        if (v == "ieee8021x" || v == "8021x")
            return KeyMgmt::Ieee8021x;

        if (v == "wpa-none")
            return KeyMgmt::WpaNone;

        if (v == "wpa-psk")
            return KeyMgmt::WpaPsk;

        if (v == "wpa-eap")
            return KeyMgmt::WpaEap;

        if (v == "sae")
            return KeyMgmt::SAE;

        if (v == "wpa-eap-suiteb-192")
            return KeyMgmt::WpaEapSuiteB192;

        if (v == "owe")
            return KeyMgmt::OWE;

        return KeyMgmt::Unknown;
    }
    QString frequencyToBand(uint freq)
    {
        if (freq <= 2484) return QStringLiteral("2.4 GHz");
        if (freq <= 5925) return QStringLiteral("5 GHz");
        return QStringLiteral("6 GHz");
    }

    QString apSecurityString(const NetworkManager::AccessPoint::Ptr &ap)
    {
        using F = NetworkManager::AccessPoint::WpaFlag;

        const auto wpa = ap->wpaFlags();
        const auto rsn = ap->rsnFlags();

        bool hasSae = rsn & F::KeyMgmtSAE;
        bool hasRsnPsk = rsn & F::KeyMgmtPsk;
        bool hasRsn8021x = rsn & F::KeyMgmt8021x;
        bool hasRsnSuiteB = rsn & F::KeyMgmtEapSuiteB192;
        bool hasOwe = rsn & (F::KeyMgmtOWE | F::KeyMgmtOWETM);
        bool hasWpaPsk = wpa & F::KeyMgmtPsk;
        bool hasWpa8021x = wpa & F::KeyMgmt8021x;
        bool hasWep = (wpa & (F::PairWep40 | F::PairWep104))
                   || (rsn & (F::GroupWep40 | F::GroupWep104));

        QStringList parts;
        if (hasSae)          parts << QStringLiteral("WPA3");
        if (hasRsnPsk || hasRsn8021x || hasRsnSuiteB) parts << QStringLiteral("WPA2");
        if (hasWpaPsk || hasWpa8021x) parts << QStringLiteral("WPA");
        if (hasOwe)          parts << QStringLiteral("OWE");

        if (!parts.isEmpty())
            return parts.join(QStringLiteral("/"));

        if (hasWep) return QStringLiteral("WEP");

        return QStringLiteral("开放");
    }

    NetworkManager::Device::Type conTypeToDeviceType(const QString &conType)
    {
        using D = NetworkManager::Device;

        if (conType == "802-11-wireless")
            return D::Wifi;
        if (conType == "802-3-ethernet")
            return D::Ethernet;

        return D::UnknownType;
    }

    QString firstInterfaceForConType(const QString &conType)
    {
        auto target = conTypeToDeviceType(conType);
        if (target == NetworkManager::Device::UnknownType)
            return {};

        for (const auto &dev : NetworkManager::networkInterfaces()) {
            if (dev && dev->type() == target)
                return dev->interfaceName();
        }
        return {};
    }

    QStringList interfacesForConType(const QString &conType)
    {
        auto target = conTypeToDeviceType(conType);
        if (target == NetworkManager::Device::UnknownType)
            return {};

        QStringList result;
        for (const auto &dev : NetworkManager::networkInterfaces()) {
            if (dev && dev->type() == target)
                result << dev->interfaceName();
        }
        return result;
    }

    QString ipv4MethodToString(NetworkManager::Ipv4Setting::ConfigMethod m)
    {
        switch (m) {
        case NetworkManager::Ipv4Setting::Automatic: return "auto";
        case NetworkManager::Ipv4Setting::Manual:    return "manual";
        case NetworkManager::Ipv4Setting::Disabled:  return "disabled";
        case NetworkManager::Ipv4Setting::LinkLocal: return "link-local";
        case NetworkManager::Ipv4Setting::Shared:    return "shared";
        default: return "auto";
        }
    }

    NetworkManager::Ipv4Setting::ConfigMethod stringToIpv4Method(const QString &s)
    {
        using M = NetworkManager::Ipv4Setting;

        if (s == "manual")     return M::Manual;
        if (s == "disabled")   return M::Disabled;
        if (s == "link-local") return M::LinkLocal;
        if (s == "shared")     return M::Shared;

        return M::Automatic;
    }

}