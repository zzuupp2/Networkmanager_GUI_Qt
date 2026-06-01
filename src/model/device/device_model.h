#pragma once

#include <QAbstractListModel>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>

#include "device_info.h"

namespace Net {

    class DeviceModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit DeviceModel(QObject *parent = nullptr);

        enum Roles {
            NameRole = Qt::UserRole + 1,
            TypeRole,
            TypeEnumRole,
            HwAddrRole,
            MtuRole,

            StateRole,
            StateSummaryRole,
            StateReasonRole,
            StateEnumRole,

            ActiveConRole,

            DriverRole,
            DriverVerRole,
            FirmwareRole,


            BitRateRole,
            CarrierRole,
            ActiveSsidRole,
            SignalRole,
        };

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

        void addDevices(const DeviceInfo &info);
        void updateDevice(const DeviceInfo &info);
        void removeDevice(QString uni);

    private:
        QList<DeviceInfo> m_allDevices;

    };
}