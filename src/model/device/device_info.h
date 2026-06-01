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

        // 驱动/硬件
        QString driver;
        QString driverVersion;
        QString firmwareVersion;


        // 状态信息
        NetworkManager::Device::State stateEnum = NetworkManager::Device::UnknownState;
        QString state;
        QString stateSummary;
        QString stateReason;

        // 当前连接
        QString activeConnection;
        QString activeConUuid;

        // 有线专有
        int bitRate = 0;   // kb/s
        bool carrier = false;

        // 无线专有
        QString activeSsid;
        int signalStrength = 0;  // %

        static DeviceInfo fromDevice(const NetworkManager::Device::Ptr &dev);
    };

}
Q_DECLARE_METATYPE(Net::DeviceInfo)