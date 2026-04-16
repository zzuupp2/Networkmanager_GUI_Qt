#include "nm_wrapper.h"

NetworkManagerWrapper::NetworkManagerWrapper(QObject *parent)
    : QObject(parent)
{
}

DeviceModel* NetworkManagerWrapper::deviceModel() {
    return &m_deviceModel;
}