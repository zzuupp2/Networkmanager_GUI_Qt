#include "device_service.h"
#include <NetworkManagerQt/Manager>

DeviceService::DeviceService(QObject *parent)
    : QObject(parent)
{
}

void DeviceService::init() {
    // 初始化已有设备
    const auto devices = NetworkManager::networkInterfaces();
    for (const auto &dev : devices) {
        QString path = dev->uni();   // ⭐ 正确：用 D-Bus path

        m_devices.insert(path, dev);

        watchDevice(dev);
        emit deviceAdded(dev);
    }

    // 监听新增
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::deviceAdded,
            this,
            &DeviceService::onDeviceAdded);

    // 监听移除
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::deviceRemoved,
            this,
            &DeviceService::onDeviceRemoved);
}

void DeviceService::onDeviceAdded(const QString &path) {
    if (m_devices.contains(path))
        return;

    auto dev = NetworkManager::findNetworkInterface(path);
    if (!dev) return;

    m_devices.insert(path, dev);

    watchDevice(dev);
    emit deviceAdded(dev);
}

void DeviceService::onDeviceRemoved(const QString &path) {
    if (!m_devices.contains(path))
        return;

    auto dev = m_devices.take(path);

    // ⭐ 清理 watched（避免残留）
    m_watchedDevices.remove(dev->uni());

    emit deviceRemoved(dev->uni());
}

void DeviceService::watchDevice(const NetworkManager::Device::Ptr &dev) {
    QString key = dev->uni();

    if (m_watchedDevices.contains(key))
        return;

    m_watchedDevices.insert(key);

    // 状态变化
    connect(dev.data(), &NetworkManager::Device::stateChanged,
            this, [this, dev]() {
                emit deviceUpdated(dev);
            });

    // IPv4变化
    connect(dev.data(), &NetworkManager::Device::ipV4ConfigChanged,
            this, [this, dev]() {
                emit deviceUpdated(dev);
            });

    // IPv6变化
    connect(dev.data(), &NetworkManager::Device::ipV6ConfigChanged,
            this, [this, dev]() {
                emit deviceUpdated(dev);
            });

    // ⭐ 补充：连接变化（很重要）
    connect(dev.data(), &NetworkManager::Device::activeConnectionChanged,
            this, [this, dev]() {
                emit deviceUpdated(dev);
            });
}

void DeviceService::start() {
    init();
}