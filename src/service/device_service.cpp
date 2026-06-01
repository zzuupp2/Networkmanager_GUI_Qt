#include "device_service.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include "src/utils/network_utils.h"

namespace Net {

DeviceService::DeviceService(QObject *parent)
    : QObject(parent) {

}

void DeviceService::init() {
    const auto devices = NetworkManager::networkInterfaces();
    for (const auto &dev : devices) {
        QString key = dev->uni();

        m_devices.insert(key, dev);

        watchDevice(dev);
        emit deviceAdded(DeviceInfo::fromDevice(dev));
    }

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::deviceAdded,
            this,
            &DeviceService::onDeviceAdded);

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

    if (m_updateTimers.contains(path)) {
        m_updateTimers[path]->deleteLater();
        m_updateTimers.remove(path);
    }

    m_watchedDevices.remove(path);
    m_frozenReasons.remove(path);

    emit deviceRemoved(path);
}

void DeviceService::watchDevice(const NetworkManager::Device::Ptr &dev) {
    QString key = dev->uni();

    if (m_watchedDevices.contains(key))
        return;

    m_watchedDevices.insert(key);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(200);

    connect(timer, &QTimer::timeout, this, [this, dev, key]() {
        auto info = DeviceInfo::fromDevice(dev);

        // 使用冻结原因覆盖瞬态
        if (m_frozenReasons.contains(key)) {
            auto frozen = m_frozenReasons[key];
            using S = NetworkManager::Device::State;
            auto st = dev->state();
            if (st != S::Activated)
                info.stateSummary = NetUtils::stateReasonSummary(st, frozen);
        }

        emit deviceUpdated(info);
    });

    m_updateTimers.insert(key, timer);

    auto scheduleUpdate = [this, key]() {
        if (m_updateTimers.contains(key))
            m_updateTimers[key]->start();
    };

    // 基础属性
    connect(dev.data(), &NetworkManager::Device::interfaceNameChanged,
            this, scheduleUpdate);
    connect(dev.data(), &NetworkManager::Device::mtuChanged,
            this, scheduleUpdate);

    // 原因冻结状态机
    connect(dev.data(), &NetworkManager::Device::stateChanged,
            this, [this, key, dev](NetworkManager::Device::State newstate,
                                    NetworkManager::Device::State oldstate,
                                    NetworkManager::Device::StateChangeReason reason) {
        // 连接成功：清除冻结
        if (newstate == NetworkManager::Device::Activated) {
            m_frozenReasons.remove(key);
        }
        // 进入断开流程且原因是有效业务原因：冻结，防止后续瞬态覆盖
        else if (!m_frozenReasons.contains(key)
                 && NetUtils::isMeaningfulReason(reason)
                 && !NetUtils::isTransientState(newstate)) {
            m_frozenReasons.insert(key, reason);
        }

        if (m_updateTimers.contains(key))
            m_updateTimers[key]->start();
    });

    connect(dev.data(), &NetworkManager::Device::activeConnectionChanged,
            this, scheduleUpdate);

    // 驱动与固件
    connect(dev.data(), &NetworkManager::Device::driverChanged,
            this, scheduleUpdate);
    connect(dev.data(), &NetworkManager::Device::driverVersionChanged,
            this, scheduleUpdate);
    connect(dev.data(), &NetworkManager::Device::firmwareVersionChanged,
            this, scheduleUpdate);

    // 有线设备专有
    if (auto wired = dev.objectCast<NetworkManager::WiredDevice>()) {
        connect(wired.data(), &NetworkManager::WiredDevice::bitRateChanged,
                this, scheduleUpdate);
        connect(wired.data(), &NetworkManager::WiredDevice::carrierChanged,
                this, scheduleUpdate);
        connect(wired.data(), &NetworkManager::WiredDevice::hardwareAddressChanged,
                this, scheduleUpdate);
    }

    // 无线设备专有
    if (auto wifi = dev.objectCast<NetworkManager::WirelessDevice>()) {
        connect(wifi.data(), &NetworkManager::WirelessDevice::activeAccessPointChanged,
                this, scheduleUpdate);
        connect(wifi.data(), &NetworkManager::WirelessDevice::hardwareAddressChanged,
                this, scheduleUpdate);
    }
}
}
