#include "device_filter_proxy_model.h"
#include "src/model/device/device_model.h"

namespace Net {

DeviceFilterProxyModel::DeviceFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void DeviceFilterProxyModel::setMode(FilterMode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;
    invalidateRowsFilter();
}

DeviceFilterProxyModel::FilterMode DeviceFilterProxyModel::mode() const
{
    return m_mode;
}

bool DeviceFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    if (!sourceModel())
        return false;

    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);

    int typeInt = sourceModel()->data(idx, DeviceModel::TypeEnumRole).toInt();

    auto type = static_cast<NetworkManager::Device::Type>(typeInt);

    auto v = classify(type);

    switch (m_mode) {
        case FilterMode::PrimaryOnly:
            return v == DeviceVisibility::Primary;

        case FilterMode::PrimaryAndSecondary:
            return v != DeviceVisibility::Hidden;

        case FilterMode::AllDevices:
            return true;
    }

    return true;
}

DeviceFilterProxyModel::DeviceVisibility
DeviceFilterProxyModel::classify(NetworkManager::Device::Type type) const
{
    using R = NetworkManager::Device::Type;

    switch (type) {

    case R::Ethernet:
    case R::Wifi:
    case R::Ppp:
    case R::Modem:
        return DeviceVisibility::Primary;

    case R::Tun:
    case R::WireGuard:
    case R::Bridge:
    case R::Bond:
    case R::Team:
    case R::Vlan:
    case R::MacVlan:
    case R::IpTunnel:
    case R::Gre:
    case R::VxLan:
        return DeviceVisibility::Secondary;

    case R::Loopback:
    case R::Veth:
    case R::Dummy:
    case R::Generic:
    case R::OvsInterface:
    case R::OvsPort:
    case R::OvsBridge:
    case R::Unused1:
    case R::Unused2:
        return DeviceVisibility::Hidden;

    default:
        return DeviceVisibility::Hidden;
    }
}
}