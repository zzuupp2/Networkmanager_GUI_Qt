#pragma once
#include <QObject>
#include <NetworkManagerQt/Settings>

#include "src/filter/device_filter_proxy_model.h"
#include "src/model/device/device_model.h"
#include "src/model/ap/ap_model.h"
// #include "src/model/connection/con_setting_model.h"
#include "src/model/connection/con_list_model.h"
#include "src/model/connection/con_runtime_model.h"
#include "src/service/ap_service.h"
#include "src/service/device_service.h"
// #include "src/service/con_setting_service.h"
#include "src/service/con_runtime_service.h"
#include "src/service/connection_manager.h"

namespace Net {

    class NetworkManagerWrapper : public QObject {
        Q_OBJECT

        Q_PROPERTY(DeviceModel* deviceModel READ deviceModel CONSTANT)
        Q_PROPERTY(QAbstractItemModel* deviceProxy READ deviceProxy CONSTANT)

        Q_PROPERTY(ApModel* apModel READ apModel CONSTANT)
        Q_PROPERTY(ApService* apService READ apService CONSTANT)
        Q_PROPERTY(ConnectionListModel* connectionListModel READ connectionListModel CONSTANT)
        Q_PROPERTY(ConnectionManager* connectionManager READ connectionManager CONSTANT)
        // Q_PROPERTY(ConnectionRuntimeModel* connectionRuntimeModel READ connectionRuntimeModel CONSTANT)

    public:
        explicit NetworkManagerWrapper(QObject *parent = nullptr);

        DeviceModel* deviceModel();
        DeviceFilterProxyModel* deviceProxy();

        ApModel* apModel();
        ApService* apService();

        ConnectionListModel* connectionListModel();
        ConnectionRuntimeModel* connectionRuntimeModel();

        ConnectionManager* connectionManager();
        // ConnectionSettingModel* connectionSettingModel();
        // QObject* apService();
        // Q_INVOKABLE void activateConnection(const QString &uuid);
        // Q_INVOKABLE void disconnectConnection(const QString &uuid);
        // Q_INVOKABLE void deleteConnection(const QString &uuid);
        // Q_INVOKABLE void connectTo(const QString &ssid);

    private:
        // void initConnectionRuntimeModel();
        void initDeviceService();
        void initConSetService();

        DeviceService m_deviceService;
        DeviceFilterProxyModel m_deviceProxy;
        DeviceModel m_deviceModel;

        ApService m_apService;
        ApModel m_apModel;

        ConnectionRuntimeService m_runtimeService;
        ConnectionRuntimeModel m_runtimeModel;
        ConnectionListModel m_connectionList;

        ConnectionManager m_connectionManager;
        // ConnectionSettingService m_conSettingService;
        // ConnectionSettingModel m_conSettingModel;
    };
}