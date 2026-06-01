#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QVariantMap>

#include "src/service/con_runtime_service.h"

namespace Net {

struct ConnectionItem
{
    QString name;
    QString uuid;
    int activeState = 0;
    QDateTime lastConnection;
};

class ConnectionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        UuidRole,
        ActiveStateRole,
        LastConnectionRole
    };

    explicit ConnectionListModel(ConnectionRuntimeService *runtime,
                                 QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString uuidAt(int row) const;
    Q_INVOKABLE bool contains(const QString &uuid) const;
    Q_INVOKABLE QVariantMap get(int row) const;
    void removeUuid(const QString &uuid);

public slots:
    void reload();

signals:
    void connectionsReloaded(const QStringList &uuids);

private slots:
    void onRuntimeChanged(const QString &uuid);

private:
    QList<ConnectionItem> m_items;
    QHash<QString, int> m_index;   // ⭐ uuid → row

    ConnectionRuntimeService *m_runtime;
};

}
