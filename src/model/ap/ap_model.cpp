#include "ap_model.h"

ApModel::ApModel(QObject *parent)
    : QAbstractListModel(parent) {
}

void ApModel::setService(ApService *service)
{
    m_service = service;

    connect(service, &ApService::accessPointsChanged,
            this, &ApModel::reload);

    connect(service, &ApService::activeApChanged,
            this, &ApModel::reload);

    reload();
}

int ApModel::rowCount(const QModelIndex &) const
{
    return m_aps.size();
}

QVariant ApModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_aps.size())
        return {};

    auto ap = m_aps[index.row()];

    // qDebug()<< "ssid: " << ap->ssid();
    // qDebug()<< "rawSsid: " << ap->rawSsid();

    switch (role) {
    case SsidRole:
        return ap->ssid();

    case StrengthRole:
        return ap->signalStrength();

    case SecurityRole: {
        bool hasWpa = ap->wpaFlags() != NetworkManager::AccessPoint::None;
        bool hasRsn = ap->rsnFlags() != NetworkManager::AccessPoint::None;
        return hasWpa || hasRsn;
    }

    case ConnectedRole:
        return (ap == m_service->activeAccessPoint());

    default:
        return {};
    }
}

QHash<int, QByteArray> ApModel::roleNames() const
{
    return {
        {SsidRole, "ssid"},
        {StrengthRole, "strength"},
        {SecurityRole, "security"},
        {ConnectedRole, "connected"}
    };
}

void ApModel::reload()
{
    if (!m_service)
        return;

    beginResetModel();

    m_aps = m_service->accessPoints();
    auto active = m_service->activeAccessPoint();

    std::sort(m_aps.begin(), m_aps.end(),
              [active](const auto &a, const auto &b) {
                  if (a == active) return true;
                  if (b == active) return false;
                  return a->signalStrength() > b->signalStrength();
              });

    endResetModel();
}