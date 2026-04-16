#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <NetworkManagerQt/Device>

class DeviceService : public QObject {
    Q_OBJECT
public:
    explicit DeviceService(QObject *parent = nullptr);
    void start();

signals:
    void deviceAdded(NetworkManager::Device::Ptr dev);
    void deviceRemoved(QString uni);
    void deviceUpdated(NetworkManager::Device::Ptr dev);

private:
    // ⭐ 用 D-Bus path 做 key（与 Notifier 一致）
    QHash<QString, NetworkManager::Device::Ptr> m_devices;

    // ⭐ 用 uni 防止重复监听
    QSet<QString> m_watchedDevices;

    void init();
    void watchDevice(const NetworkManager::Device::Ptr &dev);

private slots:
    void onDeviceAdded(const QString &path);
    void onDeviceRemoved(const QString &path);
};