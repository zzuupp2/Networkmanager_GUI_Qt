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
}