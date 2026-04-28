#include "nm_wrapper.h"
#include <QTimer>

namespace Net {

    NetworkManagerWrapper::NetworkManagerWrapper(QObject *parent)
        : QObject(parent)
    , m_runtimeService(this)
    , m_connectionList(&m_runtimeService, this)
    , m_runtimeModel(&m_runtimeService, this)
    {

        // Device proxy
        m_deviceProxy.setSourceModel(&m_deviceModel);

        // 绑定 AP service → model
        m_apModel.setService(&m_apService);

        // initConnectionRuntimeModel();

        m_editor.setConnectionManager(&m_connectionManager);
        initDeviceService();
        // initConSetService();
        // m_conService.bindModel(&m_conModel);

        QTimer::singleShot(0, this, [this]() {
            const auto first = firstConnectionUuid();
            if (!first.isEmpty()) {
                selectConnection(first);
            }
        });

    }


    DeviceModel* NetworkManagerWrapper::deviceModel() {
        return &m_deviceModel;
    }

    DeviceFilterProxyModel* NetworkManagerWrapper::deviceProxy() {
        return &m_deviceProxy;
    }

    ApModel* NetworkManagerWrapper::apModel() {
        return &m_apModel;
    }

    ApService* NetworkManagerWrapper::apService() {
        return &m_apService;
    }

    ConnectionListModel* NetworkManagerWrapper::connectionListModel() {
        return &m_connectionList;
    }

    ConnectionRuntimeModel* NetworkManagerWrapper::connectionRuntimeModel() {
        QStringList uuids;

        const auto conns = NetworkManager::listConnections();

        for (const auto &c : conns) {
            if (c)
                uuids << c->uuid();
        }

        m_runtimeModel.setConnections(uuids);
        return &m_runtimeModel;
    }

    ConnectionRuntimeModel* NetworkManagerWrapper::runtimeModel() {
        return connectionRuntimeModel();
    }

    ConnectionManager* NetworkManagerWrapper::connectionManager() {
        return &m_connectionManager;
    }

    ConnectionManager* NetworkManagerWrapper::manager() {
        return &m_connectionManager;
    }

    ConnectionEditorModel* NetworkManagerWrapper::editor() {
        return &m_editor;
    }

    QString NetworkManagerWrapper::currentUuid() const {
        return m_currentUuid;
    }

    void NetworkManagerWrapper::setCurrentUuid(const QString &uuid) {
        if (m_currentUuid == uuid)
            return;

        m_currentUuid = uuid;
        emit currentUuidChanged();
    }

    void NetworkManagerWrapper::selectConnection(const QString &uuid) {
        if (uuid.isEmpty())
            return;

        if (!hasConnection(uuid))
            return;

        setCurrentUuid(uuid);
        m_editor.loadByUuid(uuid);
    }

    QString NetworkManagerWrapper::firstConnectionUuid() const {
        if (m_connectionList.rowCount({}) <= 0)
            return {};

        return m_connectionList.uuidAt(0);
    }

    bool NetworkManagerWrapper::hasConnection(const QString &uuid) const {
        return m_connectionList.contains(uuid);
    }

    // ConnectionSettingModel* NetworkManagerWrapper::connectionSettingModel() {
    //     return &m_conSettingModel;
    // }

    // void NetworkManagerWrapper::initConnectionRuntimeModel()
    // {
    //     QStringList uuids;

    //     const auto conns = NetworkManager::listConnections();

    //     for (const auto &c : conns) {
    //         if (c)
    //             uuids << c->uuid();
    //     }

    //     m_runtimeModel.setConnections(uuids);
    // }

    void NetworkManagerWrapper::initDeviceService() {
        connect(&m_deviceService,
                &DeviceService::deviceAdded,
                &m_deviceModel,
                &DeviceModel::addDevices);

        connect(&m_deviceService,
                &DeviceService::deviceUpdated,
                &m_deviceModel,
                &DeviceModel::updateDevice);

        connect(&m_deviceService,
                &DeviceService::deviceRemoved,
                &m_deviceModel,
                &DeviceModel::removeDevice);

        m_deviceService.init();
    }

    // void NetworkManagerWrapper::initConSetService() {

    //     // connect(this, &ConnectionService::connectionAdded,
    //     //         model, &ConnectionModel::addConnection);

    //     connect(&m_conSettingService,
    //             &ConnectionSettingService::connectionRemoved,
    //             &m_conSettingModel,
    //             &ConnectionSettingModel::removeConnection);

    //     connect(&m_conSettingService,
    //             &ConnectionSettingService::connectionUpdated,
    //             &m_conSettingModel,
    //             &ConnectionSettingModel::updateConnection);

    //     // // ===== 监听 active 变化 =====
    //     // connect(NetworkManager::notifier(),
    //     //         &NetworkManager::Notifier::activeConnectionAdded,
    //     //         this, &ConnectionService::onActiveConnectionChanged);

    //     // connect(NetworkManager::notifier(),
    //     //         &NetworkManager::Notifier::activeConnectionRemoved,
    //     //         this, &ConnectionService::onActiveConnectionChanged);

    //     m_conSettingService.init();
    // }

    // ===== 操作接口 =====
    // void NetworkManagerWrapper::activateConnection(const QString &uuid)
    // {

    //     auto activting = NetworkManager::activatingConnection();
    //     if (activting) {
    //         return;
    //     }

    //     auto conn = NetworkManager::findConnectionByUuid(uuid);
    //     if (!conn || !conn->isValid()) {
    //         return;
    //     }

    //     const QString path = conn->path();

    //     if (path.isEmpty()) {
    //         return;
    //     }

    //     NetworkManager::activateConnection(path, "", "");
    // }

    // void NetworkManagerWrapper::disconnectConnection(const QString &uuid)
    // {
    //     for (const auto &acon : NetworkManager::activeConnections()) {
    //         if (acon->connection() &&
    //             acon->connection()->uuid() == uuid) {
    //             NetworkManager::deactivateConnection(acon->path());
    //         }
    //     }
    // }

}
