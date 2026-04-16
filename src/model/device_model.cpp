#include "device_model.h"
#include "device_info.h"

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_service = new DeviceService(this);

    connect(m_service, &DeviceService::deviceAdded,
            this, &DeviceModel::onDeviceAdded);

    connect(m_service, &DeviceService::deviceUpdated,
            this, &DeviceModel::onDeviceUpdated);

    connect(m_service, &DeviceService::deviceRemoved,
            this, &DeviceModel::onDeviceRemoved);

    m_service->start();
}

int DeviceModel::rowCount(const QModelIndex &) const {
    return m_devices.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_devices.size()) return {};

    const auto &dev = m_devices[index.row()];
    switch (role) {
    case NameRole: return dev.name;
    case TypeRole: return dev.type;
    case HwaddrRole: return dev.hwaddr;

    case StateRole: return dev.state;
    case StateReasonRole: return dev.stateReason;

    case Ipv4Role: return dev.ipv4;
    case Ipv4GatewayRole: return dev.ipv4Gateway;
    case Ipv4DnsRole: return dev.ipv4Dns;
    case Ipv6Role: return dev.ipv6;

    case CurConRole: return dev.activeConnection;

    case CarrierRole: return dev.carrier;
    case SpeedRole: return dev.speed;
    case SignalRole: return dev.signal;

    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    return {
            {NameRole, "devName"},
            {TypeRole, "devType"},
            {HwaddrRole, "Hwaddr"},


            {StateRole, "devState"},
            {StateReasonRole, "stateReason"},

            {Ipv4Role, "ipv4Add"},
            {Ipv4GatewayRole, "ipv4Gateway"},
            {Ipv4DnsRole, "ipv4Dns"},
            {Ipv6Role, "ipv6Add"},
            {CurConRole, "activeConnection"},

            {CarrierRole, "carrier"},
            {SpeedRole, "speed"},
            {SignalRole, "_signal"}
    };
}

void DeviceModel::onDeviceAdded(NetworkManager::Device::Ptr dev) {
    qDebug()<<"Device added !" << dev->uni();
    QString key = dev->uni();

    if (m_indexMap.contains(key))
        return;

    int row = m_devices.size();

    beginInsertRows(QModelIndex(), row, row);

    auto info = buildDeviceInfo(dev);
    m_devices.append(info);
    m_indexMap.insert(key, row);

    endInsertRows();
}

void DeviceModel::onDeviceUpdated(NetworkManager::Device::Ptr dev) {
    QString key = dev->uni();

    if (!m_indexMap.contains(key))
        return;

    int row = m_indexMap.value(key);

    m_devices[row] = buildDeviceInfo(dev);

    QModelIndex idx = index(row, 0);

    emit dataChanged(idx, idx);
}

void DeviceModel::onDeviceRemoved(QString uni) {
    if (!m_indexMap.contains(uni))
        return;

    int row = m_indexMap.value(uni);

    beginRemoveRows(QModelIndex(), row, row);

    m_devices.removeAt(row);

    endRemoveRows();

    m_indexMap.clear();
    for (int i = 0; i < m_devices.size(); ++i) {
        m_indexMap[m_devices[i].uni] = i;
    }
}