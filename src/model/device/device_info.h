#pragma once
#include <QString>
#include <QStringList>
#include <NetworkManagerQt/Manager>

namespace Net {

    struct DeviceInfo {
        // 基本信息
        QString uni;
        QString name;
        QString type;
        NetworkManager::Device::Type typeEnum;
        uint mtu;
        QString hwaddr;

        // 状态信息
        QString state;
        QString stateReason;

        QString activeConnection;
        QString activeConUuid;

        static DeviceInfo fromDevice(const NetworkManager::Device::Ptr &dev);
    };

}
Q_DECLARE_METATYPE(Net::DeviceInfo)