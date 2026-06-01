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
    QHash<QString, QTimer*> m_updateTimers;

    // 原因冻结：防止瞬态覆盖真实业务原因
    QHash<QString, NetworkManager::Device::StateChangeReason> m_frozenReasons;
};
}