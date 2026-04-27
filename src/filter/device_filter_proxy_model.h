#pragma once

#include <QObject>
#include <QSortFilterProxyModel>
#include <NetworkManagerQt/Device>

namespace Net {

class DeviceFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:

    enum class DeviceVisibility {
        Primary,
        Secondary,
        Hidden
    };
    Q_ENUM(DeviceVisibility)

    enum class FilterMode {
        PrimaryOnly,
        PrimaryAndSecondary,
        AllDevices
    };
    Q_ENUM(FilterMode)

    explicit DeviceFilterProxyModel(QObject *parent = nullptr);

    Q_INVOKABLE void setMode(FilterMode mode);
    FilterMode mode() const;

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    FilterMode m_mode = FilterMode::PrimaryOnly;

    DeviceVisibility classify(NetworkManager::Device::Type type) const;
};
}