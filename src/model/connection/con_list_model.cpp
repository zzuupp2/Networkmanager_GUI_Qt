#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>

#include "con_list_model.h"

using namespace Net;

ConnectionListModel::ConnectionListModel(ConnectionRuntimeService *runtime,
                                         QObject *parent)
    : QAbstractListModel(parent)
    , m_runtime(runtime)
{
    // ===== 监听配置变化 =====
    auto settingsNotifier = NetworkManager::settingsNotifier();

    if (settingsNotifier) {
        connect(settingsNotifier,
                &NetworkManager::SettingsNotifier::connectionAdded,
                this,
                &ConnectionListModel::reload);

        connect(settingsNotifier,
                &NetworkManager::SettingsNotifier::connectionRemoved,
                this,
                &ConnectionListModel::reload);
    }

    // ===== 监听运行时状态变化 ⭐ =====
    connect(m_runtime,
            &ConnectionRuntimeService::stateChanged,
            this,
            &ConnectionListModel::onRuntimeChanged);

    reload();
}

int ConnectionListModel::rowCount(const QModelIndex &) const
{
    return m_items.size();
}

QVariant ConnectionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto &con = m_items[index.row()];

    switch (role) {
    case NameRole: return con.name;
    case UuidRole: return con.uuid;
    case ActiveRole: return con.active;
    default: return {};
    }
}

QHash<int, QByteArray> ConnectionListModel::roleNames() const
{
    return {
        {NameRole, "conName"},
        {UuidRole, "conUuid"},
        {ActiveRole, "conActive"}
    };
}

QString ConnectionListModel::uuidAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return {};

    return m_items[row].uuid;
}

bool ConnectionListModel::contains(const QString &uuid) const
{
    return m_index.contains(uuid);
}

QVariantMap ConnectionListModel::get(int row) const
{
    if (row < 0 || row >= m_items.size())
        return {};

    const auto &item = m_items[row];
    return {
        {"conName", item.name},
        {"conUuid", item.uuid},
        {"conActive", item.active}
    };
}

void ConnectionListModel::reload()
{
    beginResetModel();

    m_items.clear();
    m_index.clear();

    const auto conns = NetworkManager::listConnections();

    int row = 0;

    for (const auto &c : conns) {
        if (!c)
            continue;

        ConnectionItem item;
        item.name = c->name();
        item.uuid = c->uuid();

        // ⭐ 使用 runtime_service
        item.active = m_runtime->state(item.uuid).active;

        m_items.append(item);
        m_index[item.uuid] = row++;
    }

    endResetModel();
}

void ConnectionListModel::onRuntimeChanged(const QString &uuid)
{
    if (!m_index.contains(uuid))
        return;

    int row = m_index[uuid];
    auto &item = m_items[row];

    auto st = m_runtime->state(uuid);

    if (item.active != st.active) {

        item.active = st.active;

        emit dataChanged(index(row), index(row),
                         {ActiveRole});
    }
}
