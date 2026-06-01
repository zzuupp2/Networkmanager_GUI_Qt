#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVariantMap>

#include "src/service/con_runtime_service.h"

namespace Net {

class ConnectionRuntimeModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int primaryRow READ primaryRow NOTIFY primaryRowChanged)
    Q_PROPERTY(int connectivity READ connectivity NOTIFY connectivityChanged)

public:
    enum Roles {
        UuidRole = Qt::UserRole + 1,
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
    };

    explicit ConnectionRuntimeModel(ConnectionRuntimeService *svc,
                                    QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap get(int row) const;

    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setConnections(const QStringList &uuids);
    void removeUuid(const QString &uuid);

    int primaryRow() const;
    int connectivity() const;
    void refreshPrimaryRow();

signals:
    void stateChanged(const QString &uuid);
    void primaryRowChanged();
    void connectivityChanged();
    void primaryChanged();

private slots:
    void onStateChanged(const QString &uuid);
    void updatePrimaryUuid();

private:
    struct Item {
        QString uuid;
        RuntimeState st;
    };
    QList<Item> m_items;
    QHash<QString, int> m_index;

    ConnectionRuntimeService *m_svc;

    QString m_primaryUuid;
    int m_connectivity = 0;
};

}
