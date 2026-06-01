#include "nm_wrapper.h"
#include "src/utils/network_utils.h"
#include <NetworkManagerQt/Manager>
#include <QDebug>
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

        // WiFi / 飞行模式 开关同步
        auto n = NetworkManager::notifier();
        connect(n, &NetworkManager::Notifier::wirelessEnabledChanged,
                this, &NetworkManagerWrapper::wirelessEnabledChanged);
        connect(n, &NetworkManager::Notifier::networkingEnabledChanged,
                this, &NetworkManagerWrapper::networkingEnabledChanged);

        // 新建连接成功后自动选中
        connect(&m_connectionManager,
                &ConnectionManager::connectionAdded,
                this,
                [this](const QString &uuid) {
            if (!uuid.isEmpty())
                selectConnection(uuid);
        });

        // 编辑同步
        connect(&m_connectionManager,
                &ConnectionManager::connectionUpdated,
                this,
                [this](const QString &uuid) {
            if (uuid == m_currentUuid)
                m_editor.loadByUuid(uuid);
        });

        // 连接更新后刷新列表（重命名等场景），NM SettingsNotifier 无 connectionUpdated 信号，
        // 因此复用我们自己的 ConnectionManager 信号
        connect(&m_connectionManager,
                &ConnectionManager::connectionUpdated,
                &m_connectionList,
                &ConnectionListModel::reload);

        connect(&m_connectionManager,
                &ConnectionManager::connectionRemoved,
                this,
                &NetworkManagerWrapper::onConnectionRemoved);

        initDeviceService();

        // ConnectionListModel 在 reload（含增删）时收集 uuid 列表并通过此信号发出，
        // 避免 wrapper 重复查询 listConnections()，消除 DBus 回调中的缓存竞态。
        connect(&m_connectionList,
                &ConnectionListModel::connectionsReloaded,
                this,
                [this](const QStringList &uuids) {
                    m_runtimeModel.setConnections(uuids);
                    m_runtimeModel.refreshPrimaryRow();
                });

        // 推迟到事件循环启动后填充运行时状态，
        // 避免 NM 异步属性尚未就绪导致 state() 返回空
        QTimer::singleShot(0, this, [this]() {
            m_runtimeService.updateAll();
            // 触发 reload，通过 connectionsReloaded 信号更新 runtime model
            m_connectionList.reload();

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

    ConnectionRuntimeModel* NetworkManagerWrapper::runtimeModel() {
        return &m_runtimeModel;
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

    QStringList NetworkManagerWrapper::interfacesForConType(const QString &conType) const
    {
        return NetUtils::interfacesForConType(conType);
    }

    bool NetworkManagerWrapper::isWirelessEnabled() const
    {
        return NetworkManager::isWirelessEnabled();
    }

    void NetworkManagerWrapper::setWirelessEnabled(bool enabled)
    {
        NetworkManager::setWirelessEnabled(enabled);
    }

    bool NetworkManagerWrapper::isNetworkingEnabled() const
    {
        return NetworkManager::isNetworkingEnabled();
    }

    void NetworkManagerWrapper::setNetworkingEnabled(bool enabled)
    {
        NetworkManager::setNetworkingEnabled(enabled);
    }

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

    void NetworkManagerWrapper::onConnectionRemoved(const QString &uuid)
    {
        qDebug() << "[onConnectionRemoved] uuid to remove:" << uuid;

        if (m_currentUuid == uuid) {
            m_currentUuid.clear();
            emit currentUuidChanged();
        }
        // 立即移除，不等待 SettingsNotifier → reload → listConnections() 刷新，
        // 避免 NM/KF6 内部缓存窗口期导致列表残留空条目。
        m_connectionList.removeUuid(uuid);
        m_runtimeModel.removeUuid(uuid);

        qDebug() << "[onConnectionRemoved] optimistic removal done, list count:"
                 << m_connectionList.rowCount({});
        qDebug() << "[onConnectionRemoved] runtime model count:"
                 << m_runtimeModel.rowCount({});
    }

}
