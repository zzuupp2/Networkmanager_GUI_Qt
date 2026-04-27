#include "ap_service.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>

ApService::ApService(QObject *parent)
    : QObject(parent)
{
    m_scanTimer.setInterval(10000); // 10s

    initWirelessDevice();

    m_lastScanTimer.start();

}

void ApService::initWirelessDevice()
{
    const auto devices = NetworkManager::networkInterfaces();

    for (const auto &dev : devices) {
        if (dev->type() == NetworkManager::Device::Wifi) {
            m_wifi = dev.objectCast<NetworkManager::WirelessDevice>();
            break;
        }
    }

    if (!m_wifi)
        return;

    connect(&m_scanTimer, &QTimer::timeout,
            this, [this](){
                requestScan(false);
            });

    // AP变化（官方信号）
    connect(m_wifi.data(),
            &NetworkManager::WirelessDevice::accessPointAppeared,
            this,
            &ApService::accessPointsChanged);

    connect(m_wifi.data(),
            &NetworkManager::WirelessDevice::accessPointDisappeared,
            this,
            &ApService::accessPointsChanged);

    // 活跃连接变化
    connect(m_wifi.data(),
            &NetworkManager::WirelessDevice::activeAccessPointChanged,
            this,
            &ApService::updateActiveAp);

    connect(m_wifi.data(),
            &NetworkManager::WirelessDevice::lastScanChanged,
            this,
            &ApService::onApChanged);

    updateActiveAp();
}

QList<NetworkManager::AccessPoint::Ptr> ApService::accessPoints() const
{
    QList<NetworkManager::AccessPoint::Ptr> result;

    if (!m_wifi)
        return result;

    const auto paths = m_wifi->accessPoints();

    for (const auto &path : paths) {
        auto ap = m_wifi->findAccessPoint(path);

        if (!ap) {
            continue;
        }

        bool ssidEmpty = ap->ssid().isEmpty();

        if (ap && !ssidEmpty)
            result.append(ap);
    }

    return result;
}

NetworkManager::AccessPoint::Ptr ApService::activeAccessPoint() const
{
    return m_activeAp;
}

void ApService::updateActiveAp()
{

    if (!m_wifi) {
        m_activeAp.clear();
        emit activeApChanged();
        return;
    }

    m_activeAp = m_wifi->activeAccessPoint();

    emit activeApChanged();
}

void ApService::onApChanged() {
    emit accessPointsChanged();
}

void ApService::requestScan(bool force) {

    if (!m_wifi || !m_scanEnable) {
        return;
    }

    if (!force  && m_lastScanTimer.elapsed() < 5000) {
        return;
    }

    m_wifi->requestScan();

    m_lastScanTimer.restart();
}

void ApService::setScanEnable(bool enable) {

    if (m_scanEnable == enable)
        return;

    m_scanEnable = enable;

    if (enable) {
        m_scanTimer.start();
        requestScan(true);
    } else {
        m_scanTimer.stop();
    }
}