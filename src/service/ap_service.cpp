#include "ap_service.h"
#include <QDebug>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessSetting>

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

    if (!m_wifi) {
        qDebug() << "[ApService] initWirelessDevice: no WiFi device found";
        return;
    }

    qDebug() << "[ApService] initWirelessDevice: WiFi device found, connecting signals";
    connect(&m_scanTimer, &QTimer::timeout,
            this, [this](){
                requestScan(false);
            });

    // AP变化
    connect(m_wifi.data(),
            &NetworkManager::WirelessDevice::accessPointAppeared,
            this,
            &ApService::accessPoin化tsChanged);

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

QString ApService::activeConnectionUuid() const
{
    return (m_wifi && m_wifi->activeConnection())
        ? m_wifi->activeConnection()->uuid() : QString();
}

QString ApService::savedConnectionUuidForSsid(const QString &ssid) const
{
    if (ssid.isEmpty())
        return {};

    for (const auto &conn : NetworkManager::listConnections()) {
        if (!conn || !conn->isValid())
            continue;

        auto settings = conn->settings();
        if (!settings)
            continue;

        if (settings->connectionType() != NetworkManager::ConnectionSettings::Wireless)
            continue;

        auto wifi = settings->setting(NetworkManager::Setting::Wireless)
                        .staticCast<NetworkManager::WirelessSetting>();
        if (wifi && wifi->ssid() == ssid)
            return conn->uuid();
    }

    return {};
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
    qDebug() << "[ApService] lastScanChanged -> accessPointsChanged, total APs:" << accessPoints().size();
    emit accessPointsChanged();
}

void ApService::requestScan(bool force) {
    qDebug() << "[ApService] requestScan(force=" << force << ") called, scanEnable=" << m_scanEnable;

    if (!m_wifi) {
        qDebug() << "[ApService] requestScan BLOCKED: no WiFi device";
        return;
    }

    if (!m_scanEnable) {
        qDebug() << "[ApService] requestScan BLOCKED: scan disabled (not on WiFi page)";
        return;
    }

    if (!force  && m_lastScanTimer.elapsed() < 5000) {
        qDebug() << "[ApService] requestScan BLOCKED: throttled, last scan was" << m_lastScanTimer.elapsed() << "ms ago (< 5000ms)";
        return;
    }

    qDebug() << "[ApService] requestScan PROCEEDING: calling NetworkManager requestScan()";
    m_wifi->requestScan();

    m_lastScanTimer.restart();
}

void ApService::setScanEnable(bool enable) {
    qDebug() << "[ApService] setScanEnable(" << enable << "), current=" << m_scanEnable;

    if (m_scanEnable == enable)
        return;

    m_scanEnable = enable;

    if (enable) {
        qDebug() << "[ApService] scan enabled: starting timer, forcing immediate scan";
        m_scanTimer.start();
        requestScan(true);
    } else {
        qDebug() << "[ApService] scan disabled: stopping timer";
        m_scanTimer.stop();
    }
}