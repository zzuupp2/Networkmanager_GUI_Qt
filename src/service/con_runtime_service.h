#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <NetworkManagerQt/ActiveConnection>

namespace Net {

struct RuntimeState
{
    int activeState = 0;   // NetworkManager::ActiveConnection::State
    int stateReason = 0;   // NetworkManager::ActiveConnection::Reason

    QString ipv4;
    QString gateway;
    QStringList dns;

    // Universal device info
    QString interface;    // e.g. eth0, wlan0
    QString mac;          // hardware address
    int mtu = 0;

    // Wired-specific
    int wiredSpeed = 0;   // Kb/s
    bool carrier = false;

    // Wireless-specific
    QString ssid;
    int wirelessRate = 0; // Kb/s
    QString mode;
    int bandwidth = 0;         // MHz
    QString frequencyBand;     // "2.4 GHz" / "5 GHz" / "6 GHz"

    QString deviceType;   // "wired" / "wireless" / ""

    bool operator==(const RuntimeState &o) const {
        return activeState == o.activeState &&
               stateReason == o.stateReason &&
               ipv4 == o.ipv4 &&
               gateway == o.gateway &&
               dns == o.dns &&
               interface == o.interface &&
               mac == o.mac &&
               mtu == o.mtu &&
               wiredSpeed == o.wiredSpeed &&
               carrier == o.carrier &&
               ssid == o.ssid &&
               wirelessRate == o.wirelessRate &&
               mode == o.mode &&
               bandwidth == o.bandwidth &&
               frequencyBand == o.frequencyBand &&
               deviceType == o.deviceType;
    }

    bool operator!=(const RuntimeState &o) const {
        return !(*this == o);
    }
};

class ConnectionRuntimeService : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionRuntimeService(QObject *parent = nullptr);

    ConnectionRuntimeService(QHash<QString, RuntimeState> m_states,
                             QSet<QString> m_boundUuids)
        : m_states(std::move(m_states)) {
    }
    void bindConnection(const QString &uuid);
    void unbindConnection(const QString &uuid);
    RuntimeState state(const QString &uuid) const;

signals:
    void stateChanged(const QString &uuid);

public:
    void updateAll();

private:
    void refreshActiveConnectionWatchers();
    void attachActiveConnection(const QString &path);
    void detachActiveConnection(const QString &path);

private slots:
    void onActiveConnectionAdded(const QString &path);
    void onActiveConnectionRemoved(const QString &path);

private:
    QHash<QString, RuntimeState> m_states;
    QHash<QString, int> m_stateReasons;
    QHash<QString, NetworkManager::ActiveConnection::Ptr> m_watchedActiveConnections;
};

}
