#pragma once
#include <QAbstractListModel>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>

#include "device_info.h"
#include "src/service/device_service.h"

class DeviceModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        HwaddrRole,

        StateRole,
        StateReasonRole,

        Ipv4Role,
        Ipv4GatewayRole,
        Ipv4DnsRole,
        Ipv6Role,

        CurConRole,

        CarrierRole,
        SpeedRole,
        SignalRole
    };

    explicit DeviceModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<DeviceInfo> m_devices;
    QHash<QString, int> m_indexMap;
    DeviceService *m_service;

    void onDeviceAdded(NetworkManager::Device::Ptr dev);
    void onDeviceUpdated(NetworkManager::Device::Ptr dev);
    void onDeviceRemoved(QString uni);

};