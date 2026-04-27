#pragma once

#include <QAbstractListModel>
#include <QList>
#include <NetworkManagerQt/AccessPoint>

#include "src/service/ap_service.h"

class ApModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ApModel(QObject *parent = nullptr);

    // 绑定 Service（数据来源）
    void setService(ApService *service);

    // QML Roles
    enum Roles {
        SsidRole = Qt::UserRole + 1,
        StrengthRole,
        SecurityRole,
        ConnectedRole
    };
    Q_ENUM(Roles)

    // Model override
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    ApService *m_service = nullptr;

    // 当前 AP 列表
    QList<NetworkManager::AccessPoint::Ptr> m_aps;

private slots:
    // 从 Service 刷新数据
    void reload();
};