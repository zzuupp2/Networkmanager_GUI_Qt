#include "con_runtime_model.h"
#include <NetworkManagerQt/Settings>
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
    case ActiveRole: return item.st.active;
    case ActivatingRole: return item.st.activating;
    case Ipv4Role: return item.st.ipv4;
    case GatewayRole: return item.st.gateway;
    case DnsRole: return item.st.dns;
    }

    return {};
}

QHash<int, QByteArray> ConnectionRuntimeModel::roleNames() const
{
    return {
        {UuidRole, "uuid"},
        {ActiveRole, "active"},
        {ActivatingRole, "activating"},
        {Ipv4Role, "ipv4"},
        {GatewayRole, "gateway"},
        {DnsRole, "dns"}
    };
}

QVariantMap ConnectionRuntimeModel::getRuntimeByUuid(const QString &uuid) const
{
    RuntimeState st;
    if (m_index.contains(uuid)) {
        st = m_items[m_index.value(uuid)].st;
    } else {
        st = m_svc->state(uuid);
    }

    return {
        {"uuid", uuid},
        {"active", st.active},
        {"activating", st.activating},
        {"ipv4", st.ipv4},
        {"gateway", st.gateway},
        {"dns", st.dns}
    };
}

void ConnectionRuntimeModel::onStateChanged(const QString &uuid)
{
    auto st = m_svc->state(uuid);

    if (!m_index.contains(uuid)) {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_index[uuid] = m_items.size();
        m_items.append({uuid, st});
        endInsertRows();
        return;
    }

    int row = m_index[uuid];
    auto &item = m_items[row];

    if (item.st != st) {
        item.st = st;
        emit dataChanged(index(row), index(row));
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
}
