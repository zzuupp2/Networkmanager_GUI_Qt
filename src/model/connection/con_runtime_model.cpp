#include "con_runtime_model.h"
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include "src/utils/network_utils.h"
using namespace Net;

ConnectionRuntimeModel::ConnectionRuntimeModel(
    ConnectionRuntimeService *svc,
    QObject *parent)
    : QAbstractListModel(parent)
    , m_svc(svc)
{
    connect(m_svc,
            &ConnectionRuntimeService::stateChanged,
            this,
            &ConnectionRuntimeModel::onStateChanged);

    auto n = NetworkManager::notifier();
    connect(n, &NetworkManager::Notifier::primaryConnectionChanged,
            this, &ConnectionRuntimeModel::updatePrimaryUuid);
    connect(n, &NetworkManager::Notifier::connectivityChanged,
            this, [this](NetworkManager::Connectivity c) {
                int v = static_cast<int>(c);
                if (m_connectivity != v) {
                    m_connectivity = v;
                    emit connectivityChanged();
                }
            });

    m_connectivity = static_cast<int>(NetworkManager::connectivity());
}

int ConnectionRuntimeModel::rowCount(const QModelIndex &) const
{
    return m_items.size();
}

QVariant ConnectionRuntimeModel::data(const QModelIndex &idx, int role) const
{
    const auto &item = m_items[idx.row()];

    switch (role) {
    case UuidRole: return item.uuid;
    case ActiveStateRole: return item.st.activeState;
    case Ipv4Role: return item.st.ipv4;
    case GatewayRole: return item.st.gateway;
    case DnsRole: return item.st.dns;
    case InterfaceRole: return item.st.interface;
    case MacRole: return item.st.mac;
    case MtuRole: return item.st.mtu;
    case WiredSpeedRole: return item.st.wiredSpeed;
    case CarrierRole: return item.st.carrier;
    case SsidRole: return item.st.ssid;
    case WirelessRateRole: return item.st.wirelessRate;
    case ModeRole: return item.st.mode;
    case BandwidthRole: return item.st.bandwidth;
    case FrequencyBandRole: return item.st.frequencyBand;
    case DeviceTypeRole: return item.st.deviceType;
    case StateReasonRole: return item.st.stateReason;
    case StatusTextRole:
        return NetUtils::activeConnectionStatusText(
            item.st.activeState, item.st.stateReason);
    }

    return {};
}

QHash<int, QByteArray> ConnectionRuntimeModel::roleNames() const
{
    return {
        {UuidRole, "uuid"},
        {ActiveStateRole, "activeState"},
        {Ipv4Role, "ipv4"},
        {GatewayRole, "gateway"},
        {DnsRole, "dns"},
        {InterfaceRole, "interface"},
        {MacRole, "mac"},
        {MtuRole, "mtu"},
        {WiredSpeedRole, "wiredSpeed"},
        {CarrierRole, "carrier"},
        {SsidRole, "ssid"},
        {WirelessRateRole, "wirelessRate"},
        {ModeRole, "mode"},
        {BandwidthRole, "bandwidth"},
        {FrequencyBandRole, "frequencyBand"},
        {DeviceTypeRole, "deviceType"},
        {StateReasonRole, "stateReason"},
        {StatusTextRole, "statusText"}
    };
}

QVariantMap ConnectionRuntimeModel::get(int row) const
{
    QVariantMap map;

    if (row < 0 || row >= m_items.size())
        return map;

    const auto &item = m_items.at(row);

    map["uuid"] = item.uuid;
    map["activeState"] = item.st.activeState;
    map["ipv4"] = item.st.ipv4;
    map["gateway"] = item.st.gateway;
    map["dns"] = item.st.dns;
    map["interface"] = item.st.interface;
    map["mac"] = item.st.mac;
    map["mtu"] = item.st.mtu;
    map["wiredSpeed"] = item.st.wiredSpeed;
    map["carrier"] = item.st.carrier;
    map["ssid"] = item.st.ssid;
    map["wirelessRate"] = item.st.wirelessRate;
    map["mode"] = item.st.mode;
    map["bandwidth"] = item.st.bandwidth;
    map["frequencyBand"] = item.st.frequencyBand;
    map["deviceType"] = item.st.deviceType;
    map["stateReason"] = item.st.stateReason;
    map["statusText"] =
        NetUtils::activeConnectionStatusText(
            item.st.activeState, item.st.stateReason);

    return map;
}

void ConnectionRuntimeModel::onStateChanged(const QString &uuid)
{
    auto st = m_svc->state(uuid);

    if (!m_index.contains(uuid)) {

        beginInsertRows(QModelIndex(),
                        m_items.size(),
                        m_items.size());

        m_index[uuid] = m_items.size();
        m_items.append({uuid, st});

        endInsertRows();

        return;
    }

    int row = m_index[uuid];
    auto &item = m_items[row];

    if (item.st != st) {

        item.st = st;

        emit dataChanged(
            index(row),
            index(row),
            {
                ActiveStateRole,
                Ipv4Role,
                GatewayRole,
                DnsRole,
                InterfaceRole,
                MacRole,
                MtuRole,
                WiredSpeedRole,
                CarrierRole,
                SsidRole,
                WirelessRateRole,
                ModeRole,
                BandwidthRole,
                FrequencyBandRole,
                DeviceTypeRole,
                StateReasonRole,
                StatusTextRole
            });

        emit stateChanged(uuid);

        if (uuid == m_primaryUuid)
            emit primaryChanged();
    }
}

void ConnectionRuntimeModel::updatePrimaryUuid()
{
    QString newUuid;
    auto ac = NetworkManager::primaryConnection();
    if (ac && ac->connection())
        newUuid = ac->connection()->uuid();

    if (m_primaryUuid != newUuid) {
        m_primaryUuid = newUuid;
        emit primaryRowChanged();
        emit primaryChanged();
    }
}

int ConnectionRuntimeModel::primaryRow() const
{
    if (m_primaryUuid.isEmpty())
        return -1;
    auto it = m_index.find(m_primaryUuid);
    return (it != m_index.end()) ? *it : -1;
}

int ConnectionRuntimeModel::connectivity() const
{
    return m_connectivity;
}

void ConnectionRuntimeModel::refreshPrimaryRow()
{
    // 主动查询 NM 当前 primary，比等待异步信号更可靠
    QString oldUuid = m_primaryUuid;
    updatePrimaryUuid();
    // 如果 UUID 没变但 model 行号变了（如 setConnections 后），补发信号
    if (m_primaryUuid == oldUuid && !m_primaryUuid.isEmpty() && m_index.contains(m_primaryUuid))
        emit primaryRowChanged();
}

void ConnectionRuntimeModel::removeUuid(const QString &uuid)
{
    if (!m_index.contains(uuid))
        return;

    int row = m_index[uuid];
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    m_index.remove(uuid);
    for (auto it = m_index.begin(); it != m_index.end(); ++it) {
        if (it.value() > row)
            it.value()--;
    }
    endRemoveRows();

    if (uuid == m_primaryUuid) {
        m_primaryUuid.clear();
        emit primaryRowChanged();
        emit primaryChanged();
    }
}

void ConnectionRuntimeModel::setConnections(const QStringList &uuids)
{
    beginResetModel();

    m_items.clear();
    m_index.clear();

    int row = 0;

    for (const auto &uuid : uuids) {

        Item item;
        item.uuid = uuid;
        item.st = m_svc->state(uuid);

        m_items.append(item);
        m_index[uuid] = row++;
    }

    endResetModel();

    // model 重建后 primary 行位置变化，必须通知 QML
    if (!m_primaryUuid.isEmpty() && m_index.contains(m_primaryUuid))
        emit primaryRowChanged();
}
