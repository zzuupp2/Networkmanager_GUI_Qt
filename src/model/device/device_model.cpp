#include "device_model.h"

namespace Net {

DeviceModel::DeviceModel(QObject *parent)
    :QAbstractListModel(parent) {

}

int DeviceModel::rowCount(const QModelIndex &) const
{
    return m_allDevices.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_allDevices.size())
        return {};

    const auto &dev = m_allDevices[index.row()];

   switch (role) {

    case NameRole: return dev.name;
    case TypeRole: return dev.type;
    case TypeEnumRole: return static_cast<int>(dev.typeEnum);
    case HwAddrRole: return dev.hwaddr;
    case MtuRole: return dev.mtu;

    case StateRole: return dev.state;
    case StateSummaryRole: return dev.stateSummary;
    case StateReasonRole: return dev.stateReason;
    case StateEnumRole: return static_cast<int>(dev.stateEnum);

    case ActiveConRole: return dev.activeConnection;

    case DriverRole: return dev.driver;
    case DriverVerRole: return dev.driverVersion;
    case FirmwareRole: return dev.firmwareVersion;


    case BitRateRole: return dev.bitRate;
    case CarrierRole: return dev.carrier;
    case ActiveSsidRole: return dev.activeSsid;
    case SignalRole: return dev.signalStrength;

    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {
        {NameRole, "devName"},
        {TypeRole, "devType"},
        {HwAddrRole, "devHwAddr"},
        {MtuRole, "devMtu"},

        {StateRole, "devState"},
        {StateSummaryRole, "devStateSummary"},
        {StateReasonRole, "stateReason"},
        {StateEnumRole, "devStateEnum"},

        {ActiveConRole, "curConnection"},

        {DriverRole, "devDriver"},
        {DriverVerRole, "devDriverVersion"},
        {FirmwareRole, "devFirmwareVersion"},


        {BitRateRole, "devBitRate"},
        {CarrierRole, "devCarrier"},
        {ActiveSsidRole, "devActiveSsid"},
        {SignalRole, "devSignalStrength"},
    };
}

void DeviceModel::addDevices(const DeviceInfo &info)
{
    QString key = info.uni;

    for (const auto &d : std::as_const(m_allDevices)) {
        if (d.uni == key)
            return;
    }

    beginInsertRows({}, m_allDevices.size(), m_allDevices.size());

    m_allDevices.append(info);

    endInsertRows();
}

void DeviceModel::updateDevice(const DeviceInfo &info)
{
    QString key = info.uni;

    for (int i = 0; i < m_allDevices.size(); ++i) {
        if (m_allDevices[i].uni == key) {

            m_allDevices[i] = info;

            QModelIndex idx = index(i);
            emit dataChanged(idx, idx);

            return;
        }
    }
}

void DeviceModel::removeDevice(QString uni)
{
    for (int i = 0; i < m_allDevices.size(); ++i) {
        if (m_allDevices[i].uni == uni) {

            beginRemoveRows({}, i, i);
            m_allDevices.removeAt(i);
            endRemoveRows();

            return;
        }
    }
}
}