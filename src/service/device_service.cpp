#include "device_service.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>

namespace Net {

DeviceService::DeviceService(QObject *parent)
    : QObject(parent) {

}

void DeviceService::init() {
    // ===== 初始化已有设备 =====
    const auto devices = NetworkManager::networkInterfaces();
    for (const auto &dev : devices) {
        QString key = dev->uni();

        m_devices.insert(key, dev);

        watchDevice(dev);
        emit deviceAdded(DeviceInfo::fromDevice(dev));
    }

    // ===== 监听新增 =====
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::deviceAdded,
            this,
            &DeviceService::onDeviceAdded);

    // ===== 监听移除 =====
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::deviceRemoved,
            this,
            &DeviceService::onDeviceRemoved);

}

void DeviceService::onDeviceAdded(const QString &path) {
    if (m_devices.contains(path))
        return;

    auto dev = NetworkManager::findNetworkInterface(path);
    if (!dev)
        return;

    m_devices.insert(path, dev);

    watchDevice(dev);
    emit deviceAdded(DeviceInfo::fromDevice(dev));
}

void DeviceService::onDeviceRemoved(const QString &path) {
    if (!m_devices.contains(path))
        return;

    auto dev = m_devices.take(path);

    // ===== 清理 timer =====
    if (m_updateTimers.contains(path)) {
        m_updateTimers[path]->deleteLater();
        m_updateTimers.remove(path);
    }

    m_watchedDevices.remove(path);

    emit deviceRemoved(path);
}

void DeviceService::watchDevice(const NetworkManager::Device::Ptr &dev) {
    QString key = dev->uni();

    if (m_watchedDevices.contains(key))
        return;

    m_watchedDevices.insert(key);

    // ===== 创建防抖 timer =====
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(200);

    connect(timer, &QTimer::timeout, this, [this, dev]() {
        emit deviceUpdated(DeviceInfo::fromDevice(dev));
    });

    m_updateTimers.insert(key, timer);

    // ===== 统一 update（重启 timer）=====
    auto update = [this, key]() {
        if (m_updateTimers.contains(key)) {
            m_updateTimers[key]->start(); // ⭐ 关键：重启 timer
        }
    };

    // =========================
    // 通用信号
    // =========================
    connect(dev.data(), &NetworkManager::Device::stateChanged,
            this, update);

    connect(dev.data(), &NetworkManager::Device::ipV4ConfigChanged,
            this, update);

    // =========================
    // 有线设备
    // =========================
    if (auto wired = dev.objectCast<NetworkManager::WiredDevice>()) {

        connect(wired.data(), &NetworkManager::WiredDevice::bitRateChanged,
                this, update);

        connect(wired.data(), &NetworkManager::WiredDevice::carrierChanged,
                this, update);
    }

    // =========================
    // WiFi设备
    // =========================
    if (auto wifi = dev.objectCast<NetworkManager::WirelessDevice>()) {

        connect(wifi.data(),
                &NetworkManager::WirelessDevice::activeAccessPointChanged,
                this, update);

        // 如果你后面要显示信号强度变化，可以加：
        // connect(wifi.data(), &NetworkManager::WirelessDevice::bitRateChanged,
        //         this, update);
    }
}

// void DeviceService::bindModel(DeviceModel *model) {
//     connect(this,
//             &DeviceService::deviceAdded,
//             model,
//             &DeviceModel::addDevices);

//     connect(this,
//             &DeviceService::deviceUpdated,
//             model,
//             &DeviceModel::updateDevice);

//     connect(this,
//             &DeviceService::deviceRemoved,
//             model,
//             &DeviceModel::removeDevice);

//     init();

// }
}