#include "con_runtime_service.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/IpConfig>

using namespace Net;

ConnectionRuntimeService::ConnectionRuntimeService(QObject *parent)
    : QObject(parent)
{
    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::activeConnectionsChanged,
            this,
            &ConnectionRuntimeService::updateAll);

    connect(NetworkManager::notifier(),
            &NetworkManager::Notifier::primaryConnectionChanged,
            this,
            &ConnectionRuntimeService::updateAll);

    updateAll();
}


RuntimeState ConnectionRuntimeService::state(const QString &uuid) const
{
    return m_states.value(uuid);
}

void ConnectionRuntimeService::updateAll()
{
    QHash<QString, RuntimeState> newStates;

    // ===== activating =====
    auto activating = NetworkManager::activatingConnection();
    if (activating && activating->isValid()) {

        const QString uuid = activating->uuid();
        RuntimeState &st = newStates[uuid];
        st.activating = true;

    }

    // ===== active =====
    for (const auto &ac : NetworkManager::activeConnections()) {

        if (!ac || !ac->isValid())
            continue;

        QString uuid = ac->uuid();
        RuntimeState st;
        st.active = true;

        auto ip4 = ac->ipV4Config();
        if (ip4.isValid()) {

            auto addrs = ip4.addresses();
            if (!addrs.isEmpty())
                st.ipv4 = addrs.first().ip().toString();

            st.gateway = ip4.gateway();

            for (const auto &addr : ip4.nameservers())
                st.dns << addr.toString();
        }

        if (newStates.contains(uuid))
            st.activating = newStates[uuid].activating;

        newStates.insert(uuid, st);
    }

    QSet<QString> changed;

    // 新增 / 更新
    for (auto it = newStates.begin(); it != newStates.end(); ++it) {
        const QString &uuid = it.key();

        if (!m_states.contains(uuid) || m_states[uuid] != it.value()) {
            m_states[uuid] = it.value();
            changed.insert(uuid);
        }
    }

    // 删除
    for (auto it = m_states.begin(); it != m_states.end();) {
        if (!newStates.contains(it.key())) {
            QString uuid = it.key();
            it = m_states.erase(it);
            changed.insert(uuid);
        } else {
            ++it;
        }
    }

    // 发信号
    for (const auto &uuid : changed)
        emit stateChanged(uuid);
}