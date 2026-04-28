#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVariantMap>

#include "src/service/con_runtime_service.h"

namespace Net {

struct ConnectionItem
{
    QString name;
    QString uuid;
    bool active = false;
};

class ConnectionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        UuidRole,
        ActiveRole
    };

    explicit ConnectionListModel(ConnectionRuntimeService *runtime,
                                 QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString uuidAt(int row) const;
    Q_INVOKABLE QVariantMap get(int row) const;

public slots:
    void reload();

private slots:
    void onRuntimeChanged(const QString &uuid);

private:
    QList<ConnectionItem> m_items;
    QHash<QString, int> m_index;   // ⭐ uuid → row

    ConnectionRuntimeService *m_runtime;
};

}
