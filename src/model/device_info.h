#pragma once
#include <QString>
#include <QStringList>
#include <NetworkManagerQt/Manager>

struct DeviceInfo {
    // 基本信息
    QString uni;
    QString name;
    QString type;
    QString hwaddr;

    // 状态信息
    QString state;
    QString stateReason;

    // 网络信息
    QString ipv4;
    QString ipv4Gateway;
    QStringList ipv4Dns;

    QString ipv6;

    QString activeConnection;

    // 物理信息
    bool carrier = false;
    int speed = 0;        // Mbps
    int signal = -1;      // WiFi: 0-100，-1 表示无
};

DeviceInfo buildDeviceInfo(const NetworkManager::Device::Ptr &dev);