#pragma once
#include <QObject>
#include "src/model/device_model.h"

class NetworkManagerWrapper : public QObject {
    Q_OBJECT
    Q_PROPERTY(DeviceModel* deviceModel READ deviceModel CONSTANT)

public:
    explicit NetworkManagerWrapper(QObject *parent = nullptr);

    DeviceModel* deviceModel();

private:
    DeviceModel m_deviceModel;
};