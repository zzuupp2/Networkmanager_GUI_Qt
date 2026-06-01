#include <QDebug>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>

#include "con_list_model.h"

using namespace Net;

ConnectionListModel::ConnectionListModel(ConnectionRuntimeService *runtime,
                                         QObject *parent)
    : QAbstractListModel(parent)
    , m_runtime(runtime)
{
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
    case ActiveStateRole: return con.activeState;
    case LastConnectionRole: return con.lastConnection;
    default: return {};
    }
}

QHash<int, QByteArray> ConnectionListModel::roleNames() const
{
    return {
        {NameRole, "conName"},
        {UuidRole, "conUuid"},
        {ActiveStateRole, "conActiveState"},
        {LastConnectionRole, "conLastConnection"}
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
        {"conActiveState", item.activeState},
        {"conLastConnection", item.lastConnection}
    };
}

void ConnectionListModel::reload()
{
    beginResetModel();

    m_items.clear();
    m_index.clear();

    const auto conns = NetworkManager::listConnections();

    int row = 0;
    QStringList uuids;

    for (const auto &c : conns) {
        if (!c)
            continue;

        // NM 删除连接时存在过渡态：name 和 uuid 可能已被清空但对象仍出现在列表中，
        // 过滤掉此类半删除条目，避免 QML 列表残留无信息的占位项。
        const QString uuid = c->uuid();
        if (uuid.isEmpty())
            continue;

        ConnectionItem item;
        item.name = c->name();
        item.uuid = uuid;

        item.activeState = m_runtime->state(item.uuid).activeState;
        // lastConnection 由 onRuntimeChanged 在连接进入 Activated 状态时设置，
        // 不依赖 NM 的 timestamp（NM 在 AddConnection 时就会写入创建时间，无法区分"新建"和"激活过"）

        m_items.append(item);
        m_index[item.uuid] = row++;

        uuids << item.uuid;
    }

    endResetModel();

    qDebug() << "[ConListModel::reload] NM listConnections() returned" << uuids.size()
             << "connections:";
    for (const auto &item : m_items) {
        qDebug() << "  name:" << item.name << "uuid:" << item.uuid
                 << "activeState:" << item.activeState;
    }

    emit connectionsReloaded(uuids);
}

void ConnectionListModel::removeUuid(const QString &uuid)
{
    if (!m_index.contains(uuid)) {
        qDebug() << "[ConListModel::removeUuid] uuid not in index:" << uuid
                 << "current count:" << m_items.size();
        return;
    }

    int row = m_index[uuid];
    qDebug() << "[ConListModel::removeUuid] removing row:" << row
             << "uuid:" << uuid << "name:" << m_items[row].name;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    m_index.remove(uuid);
    // 修正被删除行之后的所有索引
    for (auto it = m_index.begin(); it != m_index.end(); ++it) {
        if (it.value() > row)
            it.value()--;
    }
    endRemoveRows();

    qDebug() << "[ConListModel::removeUuid] done, remaining count:" << m_items.size();
}

void ConnectionListModel::onRuntimeChanged(const QString &uuid)
{
    if (!m_index.contains(uuid))
        return;

    int row = m_index[uuid];
    auto &item = m_items[row];

    auto st = m_runtime->state(uuid);

    if (item.activeState != st.activeState) {

        item.activeState = st.activeState;

        // 进入 Activated 状态时记录激活时间
        if (st.activeState == 2)  // Activated
            item.lastConnection = QDateTime::currentDateTime();

        emit dataChanged(index(row), index(row),
                         {ActiveStateRole, LastConnectionRole});
    }
}
