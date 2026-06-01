#include "con_editor_model.h"
#include "src/service/connection_manager.h"
#include "src/utils/network_utils.h"

#include <QUuid>

namespace Net {

ConnectionEditorModel::ConnectionEditorModel(QObject *parent)
    : QObject(parent)
{
}

void ConnectionEditorModel::loadFromSettingInfo(const ConnectionSettingInfo &info)
{
    m_original = info;
    m_working = info;

    if (m_isNew) {
        m_isNew = true;
        emit isNewChanged(false);
    }

    m_isModified = false;

    m_isModified = false;
    emit isModifiedChanged(false);

    emit idChanged();
    emit uuidChanged();
    emit typeChanged();

    emit autoconnectChanged();
    emit autoconnectPriorityChanged();
    emit interfaceNameChanged();

    emit ssidChanged();
    emit wirelessSecurityChanged();
    emit wirelessPasswordChanged();

    emit mtuChanged();
    emit wiredAutoNegotiateChanged();
    emit wiredSpeedChanged();
    emit wiredDuplexChanged();

    emit ipv4MethodChanged();
    emit ipv4AddressChanged();
    emit ipv4GatewayChanged();
    emit ipv4DnsChanged();
}

void ConnectionEditorModel::loadDefaults(const QString &type)
{
    ConnectionSettingInfo info;
    info.type = type;
    info.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    info.autoconnect = true;
    info.ipv4Method = "auto";

    if (type == "802-11-wireless") {
        info.id = "New Wi-Fi";
        info.wirelessSecurity = "wpa-psk";
    } else if (type == "802-3-ethernet") {
        info.id = "New Wired";
    }

    info.interfaceName = NetUtils::firstInterfaceForConType(type);

    loadFromSettingInfo(info);

    m_isNew = true;
    m_isModified = true;
    emit isNewChanged(true);
    emit isModifiedChanged(true);
}

void ConnectionEditorModel::setConnectionManager(ConnectionManager *manager)
{
    m_manager = manager;
}

bool ConnectionEditorModel::loadByUuid(const QString &uuid)
{
    if (!m_manager) return false;

    auto info = m_manager->getConnectionSettingInfo(uuid);
    if (info.uuid.isEmpty()) return false;

    loadFromSettingInfo(info);

    if (m_isNew) {
        m_isNew = false;
        emit isNewChanged(false);
    }
    return true;
}

void ConnectionEditorModel::reset()
{
    m_working = m_original;

    m_isModified = false;
    emit isModifiedChanged(false);

    emit editRejected();

    emit idChanged();
    emit uuidChanged();
    emit typeChanged();

    emit autoconnectChanged();
    emit autoconnectPriorityChanged();
    emit interfaceNameChanged();

    emit ssidChanged();
    emit wirelessSecurityChanged();
    emit wirelessPasswordChanged();

    emit mtuChanged();
    emit wiredAutoNegotiateChanged();
    emit wiredSpeedChanged();
    emit wiredDuplexChanged();

    emit ipv4MethodChanged();
    emit ipv4AddressChanged();
    emit ipv4GatewayChanged();
    emit ipv4DnsChanged();
}

void ConnectionEditorModel::commit()
{
    m_original = m_working;

    m_isModified = false;
    m_isNew = false;

    emit isModifiedChanged(false);
    emit isNewChanged(false);
}

QVariantMap ConnectionEditorModel::toPatch() const
{
    QVariantMap patch;

    if (m_working.id != m_original.id)
        patch["id"] = m_working.id;

    if (m_working.autoconnect != m_original.autoconnect)
        patch["autoconnect"] = m_working.autoconnect;

    if (m_working.autoconnectPriority != m_original.autoconnectPriority)
        patch["autoconnectPriority"] = m_working.autoconnectPriority;

    if (m_working.interfaceName != m_original.interfaceName)
        patch["interfaceName"] = m_working.interfaceName;

    if (m_working.ssid != m_original.ssid)
        patch["ssid"] = m_working.ssid;

    if (m_working.wirelessSecurity != m_original.wirelessSecurity)
        patch["wirelessSecurity"] = m_working.wirelessSecurity;

    if (m_working.wirelessPassword != m_original.wirelessPassword)
        patch["wirelessPassword"] = m_working.wirelessPassword;

    if (m_working.mtu != m_original.mtu)
        patch["mtu"] = m_working.mtu;

    if (m_working.wiredAutoNegotiate != m_original.wiredAutoNegotiate)
        patch["wiredAutoNegotiate"] = m_working.wiredAutoNegotiate;

    if (m_working.wiredSpeed != m_original.wiredSpeed)
        patch["wiredSpeed"] = m_working.wiredSpeed;

    if (m_working.wiredDuplex != m_original.wiredDuplex)
        patch["wiredDuplex"] = m_working.wiredDuplex;

    if (m_working.ipv4Method != m_original.ipv4Method)
        patch["ipv4Method"] = m_working.ipv4Method;

    if (m_working.ipv4Address != m_original.ipv4Address)
        patch["ipv4Address"] = m_working.ipv4Address;

    if (m_working.ipv4Gateway != m_original.ipv4Gateway)
        patch["ipv4Gateway"] = m_working.ipv4Gateway;

    if (m_working.ipv4Dns != m_original.ipv4Dns)
        patch["ipv4Dns"] = m_working.ipv4Dns;

    return patch;
}

void ConnectionEditorModel::setId(const QString &v)
{
    if (m_working.id == v) return;
    m_working.id = v;
    emit idChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setAutoconnect(bool v)
{
    if (m_working.autoconnect == v) return;
    m_working.autoconnect = v;
    emit autoconnectChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setAutoconnectPriority(int v)
{
    if (m_working.autoconnectPriority == v) return;
    m_working.autoconnectPriority = v;
    emit autoconnectPriorityChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setInterfaceName(const QString &v)
{
    if (m_working.interfaceName == v) return;
    m_working.interfaceName = v;
    emit interfaceNameChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setSsid(const QString &v)
{
    if (m_working.ssid == v) return;
    m_working.ssid = v;
    emit ssidChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setWirelessSecurity(const QString &v)
{
    if (m_working.wirelessSecurity == v) return;
    m_working.wirelessSecurity = v;
    emit wirelessSecurityChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setWirelessPassword(const QString &v)
{
    if (m_working.wirelessPassword == v) return;
    m_working.wirelessPassword = v;
    emit wirelessPasswordChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setMtu(int v)
{
    if (m_working.mtu == v) return;
    m_working.mtu = v;
    emit mtuChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setWiredAutoNegotiate(int v)
{
    if (m_working.wiredAutoNegotiate == v) return;
    m_working.wiredAutoNegotiate = v;
    emit wiredAutoNegotiateChanged();
    // 切换模式时重置 speed/duplex
    if (v != 0) {
        if (m_working.wiredSpeed != 0) {
            m_working.wiredSpeed = 0;
            emit wiredSpeedChanged();
        }
        if (!m_working.wiredDuplex.isEmpty()) {
            m_working.wiredDuplex.clear();
            emit wiredDuplexChanged();
        }
    }
    updateModificationFlag();
}

void ConnectionEditorModel::setWiredSpeed(int v)
{
    if (m_working.wiredSpeed == v) return;
    m_working.wiredSpeed = v;
    emit wiredSpeedChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setWiredDuplex(const QString &v)
{
    if (m_working.wiredDuplex == v) return;
    m_working.wiredDuplex = v;
    emit wiredDuplexChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setIpv4Method(const QString &v)
{
    if (m_working.ipv4Method == v) return;
    m_working.ipv4Method = v;
    emit ipv4MethodChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setIpv4Address(const QString &v)
{
    if (m_working.ipv4Address == v) return;
    m_working.ipv4Address = v;
    emit ipv4AddressChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setIpv4Gateway(const QString &v)
{
    if (m_working.ipv4Gateway == v) return;
    m_working.ipv4Gateway = v;
    emit ipv4GatewayChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::setIpv4Dns(const QStringList &v)
{
    if (m_working.ipv4Dns == v) return;
    m_working.ipv4Dns = v;
    emit ipv4DnsChanged();
    updateModificationFlag();
}

void ConnectionEditorModel::updateModificationFlag()
{
    bool modified =
        (m_working.id != m_original.id) ||
        (m_working.autoconnect != m_original.autoconnect) ||
        (m_working.autoconnectPriority != m_original.autoconnectPriority) ||
        (m_working.interfaceName != m_original.interfaceName) ||
        (m_working.ssid != m_original.ssid) ||
        (m_working.wirelessSecurity != m_original.wirelessSecurity) ||
        (m_working.wirelessPassword != m_original.wirelessPassword) ||
        (m_working.mtu != m_original.mtu) ||
        (m_working.wiredAutoNegotiate != m_original.wiredAutoNegotiate) ||
        (m_working.wiredSpeed != m_original.wiredSpeed) ||
        (m_working.wiredDuplex != m_original.wiredDuplex) ||
        (m_working.ipv4Method != m_original.ipv4Method) ||
        (m_working.ipv4Address != m_original.ipv4Address) ||
        (m_working.ipv4Gateway != m_original.ipv4Gateway) ||
        (m_working.ipv4Dns != m_original.ipv4Dns);

    if (m_isModified != modified) {
        m_isModified = modified;
        emit isModifiedChanged(modified);
    }
}

QString ConnectionEditorModel::id() const { return m_working.id; }
QString ConnectionEditorModel::uuid() const { return m_working.uuid; }
QString ConnectionEditorModel::type() const { return m_working.type; }

bool ConnectionEditorModel::autoconnect() const { return m_working.autoconnect; }
int ConnectionEditorModel::autoconnectPriority() const { return m_working.autoconnectPriority; }
QString ConnectionEditorModel::interfaceName() const { return m_working.interfaceName; }

QString ConnectionEditorModel::ssid() const { return m_working.ssid; }
QString ConnectionEditorModel::wirelessSecurity() const { return m_working.wirelessSecurity; }
QString ConnectionEditorModel::wirelessPassword() const { return m_working.wirelessPassword; }

int ConnectionEditorModel::mtu() const { return m_working.mtu; }

int ConnectionEditorModel::wiredAutoNegotiate() const { return m_working.wiredAutoNegotiate; }
int ConnectionEditorModel::wiredSpeed() const { return m_working.wiredSpeed; }
QString ConnectionEditorModel::wiredDuplex() const { return m_working.wiredDuplex; }

QString ConnectionEditorModel::ipv4Method() const { return m_working.ipv4Method; }
QString ConnectionEditorModel::ipv4Address() const { return m_working.ipv4Address; }
QString ConnectionEditorModel::ipv4Gateway() const { return m_working.ipv4Gateway; }
QStringList ConnectionEditorModel::ipv4Dns() const { return m_working.ipv4Dns; }

bool ConnectionEditorModel::isModified() const { return m_isModified; }
bool ConnectionEditorModel::isNew() const { return m_isNew; }

ConnectionSettingInfo ConnectionEditorModel::toSettingInfo() const
{
    return m_working;
}

} // namespace Net
