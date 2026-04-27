#pragma once
#include <QString>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>

namespace NetUtils {
    QString deviceTypeToString(NetworkManager::Device::Type type);
    QString deviceStateToString(NetworkManager::Device::State state);
    QString stateReasonToString(NetworkManager::Device::StateChangeReason reason);

    QString getHwAddr(const NetworkManager::Device::Ptr &dev);

    bool isDeviceType(const NetworkManager::Device::Ptr &dev,
                      NetworkManager::Device::Type devType);
    bool isNetworkDevice(const NetworkManager::Device::Ptr &dev);
    bool isWifiDev(const NetworkManager::Device::Ptr &dev);
    bool isEthernetDev(const NetworkManager::Device::Ptr &dev);

    bool isConSetType(const NetworkManager::ConnectionSettings &set,
                              NetworkManager::ConnectionSettings::ConnectionType conType);

    bool isWiredConType(const NetworkManager::ConnectionSettings::Ptr &set);
    bool isWirelessConType(const NetworkManager::ConnectionSettings::Ptr &set);

    bool isConActive(const QString &uuid);
}