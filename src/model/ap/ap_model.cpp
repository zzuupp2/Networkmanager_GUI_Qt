#include "ap_model.h"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Settings>
#include "src/utils/network_utils.h"

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

    case BandRole:
        return NetUtils::frequencyToBand(ap->frequency());

    case BssidRole:
        return ap->hardwareAddress();

    case SecurityTypeRole:
        return NetUtils::apSecurityString(ap);

    case BandwidthRole:
        return static_cast<int>(ap->bandwidth());

    case ActiveConnectionUuidRole:
        return m_service ? m_service->activeConnectionUuid() : QString();

    case SavedConnectionUuidRole:
        return m_ssidToSavedUuid.value(ap->ssid());

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
        {ConnectedRole, "connected"},
        {BandRole, "band"},
        {BssidRole, "bssid"},
        {SecurityTypeRole, "securityType"},
        {BandwidthRole, "bandwidth"},
        {ActiveConnectionUuidRole, "activeConnectionUuid"},
        {SavedConnectionUuidRole, "savedConnectionUuid"}
    };
}

void ApModel::reload()
{
    if (!m_service)
        return;

    beginResetModel();

    // 构建 SSID→UUID 缓存
    m_ssidToSavedUuid.clear();
    for (const auto &conn : NetworkManager::listConnections()) {
        if (!conn || !conn->isValid())
            continue;
        auto settings = conn->settings();
        if (!settings || settings->connectionType() != NetworkManager::ConnectionSettings::Wireless)
            continue;
        auto wifi = settings->setting(NetworkManager::Setting::Wireless)
                        .staticCast<NetworkManager::WirelessSetting>();
        if (wifi && !wifi->ssid().isEmpty())
            m_ssidToSavedUuid[wifi->ssid()] = conn->uuid();
    }

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