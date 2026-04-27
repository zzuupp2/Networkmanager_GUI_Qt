#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QTimer>

#include <NetworkManagerQt/Device>
#include <src/model/device/device_info.h>

namespace Net {


class DeviceService : public QObject
{
    Q_OBJECT

public:
    explicit DeviceService(QObject *parent = nullptr);
    void init();

signals:
    void deviceAdded(DeviceInfo info);
    void deviceRemoved(QString uni);
    void deviceUpdated(DeviceInfo info);

private:

    void onDeviceAdded(const QString &path);
    void onDeviceRemoved(const QString &path);
    void watchDevice(const NetworkManager::Device::Ptr &dev);

private:
    QHash<QString, NetworkManager::Device::Ptr> m_devices;
    QSet<QString> m_watchedDevices;

    // ⭐ 每个设备一个防抖 timer
    QHash<QString, QTimer*> m_updateTimers;
};
}