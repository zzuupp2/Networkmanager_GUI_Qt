#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVariantMap>

#include "src/service/con_runtime_service.h"

namespace Net {

class ConnectionRuntimeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        UuidRole = Qt::UserRole + 1,
        ActiveRole,
        ActivatingRole,
        Ipv4Role,
        GatewayRole,
        DnsRole
    };

    explicit ConnectionRuntimeModel(ConnectionRuntimeService *svc,
                                    QObject *parent = nullptr);

    void setConnections(const QStringList &uuids);

    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap getRuntimeByUuid(const QString &uuid) const;

private slots:
    void onStateChanged(const QString &uuid);

private:
    struct Item {
        QString uuid;
        RuntimeState st;
    };
    QList<Item> m_items;
    QHash<QString, int> m_index;

    ConnectionRuntimeService *m_svc;
};

}
