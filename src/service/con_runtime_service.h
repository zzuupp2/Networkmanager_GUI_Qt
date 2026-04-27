#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QStringList>

namespace Net {

struct RuntimeState
{
    bool active = false;
    bool activating = false;

    QString ipv4;
    QString gateway;
    QStringList dns;

    bool operator==(const RuntimeState &o) const {
        return active == o.active &&
               activating == o.activating &&
               ipv4 == o.ipv4 &&
               gateway == o.gateway &&
               dns == o.dns;
    }

    bool operator!=(const RuntimeState &o) const {
        return !(*this == o);
    }
};

// ✅ 类名已改为 ConnectionRuntimeService
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

private:
    void updateAll();

private:
    QHash<QString, RuntimeState> m_states;
};

}