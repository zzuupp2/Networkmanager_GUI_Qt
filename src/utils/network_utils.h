#pragma once
#include <QString>
#include <QStringList>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Ipv4Setting>

namespace NetUtils {
    QString deviceTypeToString(NetworkManager::Device::Type type);
    QString deviceStateToString(NetworkManager::Device::State state);
    QString stateReasonToString(NetworkManager::Device::StateChangeReason reason);
    QString stateReasonSummary(NetworkManager::Device::State state, NetworkManager::Device::StateChangeReason reason);
    QString stateReasonSummary(NetworkManager::Device::State oldState,
                               NetworkManager::Device::State newState,
                               NetworkManager::Device::StateChangeReason reason);
    bool isMeaningfulReason(NetworkManager::Device::StateChangeReason reason);
    bool isTransientState(NetworkManager::Device::State state);

    QString getHwAddr(const NetworkManager::Device::Ptr &dev);

    bool isDeviceType(const NetworkManager::Device::Ptr &dev,
                      NetworkManager::Device::Type devType);
    bool isNetworkDevice(const NetworkManager::Device::Ptr &dev);
    bool isWifiDev(const NetworkManager::Device::Ptr &dev);
    bool isEthernetDev(const NetworkManager::Device::Ptr &dev);

    bool isConSetType(const NetworkManager::ConnectionSettings::Ptr &set,
                        NetworkManager::ConnectionSettings::ConnectionType conType);

    bool isWiredConType(const NetworkManager::ConnectionSettings::Ptr &set);
    bool isWirelessConType(const NetworkManager::ConnectionSettings::Ptr &set);

    bool isConActive(const QString &uuid);

    QString activeConnectionStateToString(int state);
    QString activeConnectionReasonToString(int reason);
    QString activeConnectionStatusText(int activeState, int stateReason);

    QString keyMgmtToString(const NetworkManager::WirelessSecuritySetting::KeyMgmt k);
    NetworkManager::WirelessSecuritySetting::KeyMgmt stringToKeyMgmt(const QString &s);

    QString frequencyToBand(uint freq);
    QString apSecurityString(const NetworkManager::AccessPoint::Ptr &ap);

    NetworkManager::Device::Type conTypeToDeviceType(const QString &conType);
    QString firstInterfaceForConType(const QString &conType);
    QStringList interfacesForConType(const QString &conType);

    QString ipv4MethodToString(NetworkManager::Ipv4Setting::ConfigMethod m);
    NetworkManager::Ipv4Setting::ConfigMethod stringToIpv4Method(const QString &s);
}
