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
    case MtuRole: return dev.mtu;

    case StateRole: return dev.state;
    case StateReasonRole: return dev.stateReason;

    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {
        {NameRole, "devName"},
        {TypeRole, "devType"},
        {MtuRole, "devMtu"},

        {StateRole, "devState"},
        {StateReasonRole, "stateReason"},
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