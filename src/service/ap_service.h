#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/AccessPoint>

class ApService : public QObject {
    Q_OBJECT

public:
    explicit ApService(QObject *parent = nullptr);

    QList<NetworkManager::AccessPoint::Ptr> accessPoints() const;
    NetworkManager::AccessPoint::Ptr activeAccessPoint() const;

    Q_INVOKABLE void requestScan(bool force);
    Q_INVOKABLE void setScanEnable(bool enable);

signals:
    void accessPointsChanged();
    void activeApChanged();

private:
    QTimer m_scanTimer;
    QElapsedTimer m_lastScanTimer;
    bool m_scanEnable = false;

    NetworkManager::WirelessDevice::Ptr m_wifi;
    NetworkManager::AccessPoint::Ptr m_activeAp;

    void onApChanged();
    void initWirelessDevice();
    void updateActiveAp();

};