#include <QDebug>
#include <QMetaType>

#include "con_editor_model.h"
namespace Net {

ConnectionEditorModel::ConnectionEditorModel(QObject *parent)
    : QObject(parent)
    , m_isModified(false)
    , m_isNew(false)
{
}

// ========== 数据加载 ==========

void ConnectionEditorModel::loadFromSettingInfo(const ConnectionSettingInfo &info)
{
    const auto oldUuid = m_working.uuid;
    const auto oldType = m_working.type;

    m_original = info;
    m_working = info;
    if (m_isNew) {
        m_isNew = false;
        emit isNewChanged(false);
    }
    m_isModified = false;

    // 通知所有属性变更，触发 QML 界面完全刷新
    emit idChanged();
    if (oldUuid != m_working.uuid)
        emit uuidChanged();
    if (oldType != m_working.type)
        emit typeChanged();
    emit autoconnectChanged();
    emit autoconnectPriorityChanged();
    emit interfaceNameChanged();
    emit ssidChanged();
    emit wirelessSecurityChanged();
    emit wirelessPasswordChanged();
    emit mtuChanged();
    emit ipv4MethodChanged();
    emit ipv4AddressChanged();
    emit ipv4GatewayChanged();
    emit ipv4DnsChanged();
    // emit ipv6MethodChanged();
    // emit ipv6AddressChanged();
    // emit ipv6GatewayChanged();
    // emit ipv6DnsChanged();
    emit isModifiedChanged(false);
}

void ConnectionEditorModel::loadDefaults(const QString &type)
{
    ConnectionSettingInfo info;
    info.type = type;
    // 根据类型填充默认名
    if (type == "802-11-wireless") {
        info.id = "New Wi-Fi Connection";
        info.wirelessSecurity = "wpa-psk";   // 默认安全协议
        info.ssid = "";
    } else if (type == "802-3-ethernet") {
        info.id = "New Wired Connection";
        info.mtu = 0;   // 自动
    } else {
        info.id = "New Connection";
    }

    info.autoconnect = true;
    info.autoconnectPriority = 0;
    info.ipv4Method = "auto";
    // info.ipv6Method = "auto";
    info.interfaceName = "";

    loadFromSettingInfo(info);
    if (!m_isNew) {
        m_isNew = true;
        emit isNewChanged(true);
    }
}

// ========== 操作 ==========

void ConnectionEditorModel::reset()
{
    m_working = m_original;
    m_isModified = false;

    // 刷新所有属性到原始值
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
    emit ipv4MethodChanged();
    emit ipv4AddressChanged();
    emit ipv4GatewayChanged();
    emit ipv4DnsChanged();
    // emit ipv6MethodChanged();
    // emit ipv6AddressChanged();
    // emit ipv6GatewayChanged();
    // emit ipv6DnsChanged();
    emit isModifiedChanged(false);

    emit editRejected();
}

bool ConnectionEditorModel::setField(const QString &field, const QVariant &value)
{
    if (field == "id") {
        setId(value.toString());
        return true;
    }
    if (field == "autoconnect") {
        setAutoconnect(value.toBool());
        return true;
    }
    if (field == "autoconnectPriority") {
        setAutoconnectPriority(value.toInt());
        return true;
    }
    if (field == "interfaceName") {
        setInterfaceName(value.toString());
        return true;
    }
    if (field == "ssid") {
        setSsid(value.toString());
        return true;
    }
    if (field == "wirelessSecurity") {
        setWirelessSecurity(value.toString());
        return true;
    }
    if (field == "wirelessPassword") {
        setWirelessPassword(value.toString());
        return true;
    }
    if (field == "mtu") {
        setMtu(value.toInt());
        return true;
    }
    if (field == "ipv4Method") {
        setIpv4Method(value.toString());
        return true;
    }
    if (field == "ipv4Address") {
        setIpv4Address(value.toString());
        return true;
    }
    if (field == "ipv4Gateway") {
        setIpv4Gateway(value.toString());
        return true;
    }
    if (field == "ipv4Dns") {
        if (value.canConvert<QStringList>()) {
            setIpv4Dns(value.toStringList());
            return true;
        }

        if (value.typeId() == QMetaType::QString) {
            setIpv4Dns(value.toString().split(',', Qt::SkipEmptyParts));
            return true;
        }
    }

    return false;
}

void ConnectionEditorModel::applyPatch(const QVariantMap &patch)
{
    for (auto it = patch.constBegin(); it != patch.constEnd(); ++it) {
        setField(it.key(), it.value());
    }
}

ConnectionSettingInfo ConnectionEditorModel::toSettingInfo() const
{
    return m_working;
}

QVariantMap ConnectionEditorModel::toSettingsMap() const
{
    return m_working.toNMSettings();
}

// ========== 属性访问器 ==========

QString ConnectionEditorModel::id() const { return m_working.id; }
void ConnectionEditorModel::setId(const QString &id) {
    if (m_working.id == id) return;
    m_working.id = id;
    emit idChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::uuid() const { return m_working.uuid; }
QString ConnectionEditorModel::type() const { return m_working.type; }

bool ConnectionEditorModel::autoconnect() const { return m_working.autoconnect; }
void ConnectionEditorModel::setAutoconnect(bool enabled) {
    if (m_working.autoconnect == enabled) return;
    m_working.autoconnect = enabled;
    emit autoconnectChanged();
    updateModificationFlag();
}

int ConnectionEditorModel::autoconnectPriority() const { return m_working.autoconnectPriority; }
void ConnectionEditorModel::setAutoconnectPriority(int priority) {
    if (m_working.autoconnectPriority == priority) return;
    m_working.autoconnectPriority = priority;
    emit autoconnectPriorityChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::interfaceName() const { return m_working.interfaceName; }
void ConnectionEditorModel::setInterfaceName(const QString &iface) {
    if (m_working.interfaceName == iface) return;
    m_working.interfaceName = iface;
    emit interfaceNameChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::ssid() const { return m_working.ssid; }
void ConnectionEditorModel::setSsid(const QString &ssid) {
    if (m_working.ssid == ssid) return;
    m_working.ssid = ssid;
    emit ssidChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::wirelessSecurity() const { return m_working.wirelessSecurity; }
void ConnectionEditorModel::setWirelessSecurity(const QString &security) {
    if (m_working.wirelessSecurity == security) return;
    m_working.wirelessSecurity = security;
    emit wirelessSecurityChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::wirelessPassword() const { return m_working.wirelessPassword; }
void ConnectionEditorModel::setWirelessPassword(const QString &password) {
    if (m_working.wirelessPassword == password) return;
    m_working.wirelessPassword = password;
    emit wirelessPasswordChanged();
    updateModificationFlag();
}

int ConnectionEditorModel::mtu() const { return m_working.mtu; }
void ConnectionEditorModel::setMtu(int mtu) {
    if (m_working.mtu == mtu) return;
    m_working.mtu = mtu;
    emit mtuChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::ipv4Method() const { return m_working.ipv4Method; }
void ConnectionEditorModel::setIpv4Method(const QString &method) {
    if (m_working.ipv4Method == method) return;
    m_working.ipv4Method = method;
    emit ipv4MethodChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::ipv4Address() const { return m_working.ipv4Address; }
void ConnectionEditorModel::setIpv4Address(const QString &address) {
    if (m_working.ipv4Address == address) return;
    m_working.ipv4Address = address;
    emit ipv4AddressChanged();
    updateModificationFlag();
}

QString ConnectionEditorModel::ipv4Gateway() const { return m_working.ipv4Gateway; }
void ConnectionEditorModel::setIpv4Gateway(const QString &gateway) {
    if (m_working.ipv4Gateway == gateway) return;
    m_working.ipv4Gateway = gateway;
    emit ipv4GatewayChanged();
    updateModificationFlag();
}

QStringList ConnectionEditorModel::ipv4Dns() const { return m_working.ipv4Dns; }
void ConnectionEditorModel::setIpv4Dns(const QStringList &dns) {
    if (m_working.ipv4Dns == dns) return;
    m_working.ipv4Dns = dns;
    emit ipv4DnsChanged();
    updateModificationFlag();
}

// QString ConnectionEditorModel::ipv6Method() const { return m_working.ipv6Method; }
// void ConnectionEditorModel::setIpv6Method(const QString &method) {
//     if (m_working.ipv6Method == method) return;
//     m_working.ipv6Method = method;
//     emit ipv6MethodChanged();
//     updateModificationFlag();
// }

// QString ConnectionEditorModel::ipv6Address() const { return m_working.ipv6Address; }
// void ConnectionEditorModel::setIpv6Address(const QString &address) {
//     if (m_working.ipv6Address == address) return;
//     m_working.ipv6Address = address;
//     emit ipv6AddressChanged();
//     updateModificationFlag();
// }

// QString ConnectionEditorModel::ipv6Gateway() const { return m_working.ipv6Gateway; }
// void ConnectionEditorModel::setIpv6Gateway(const QString &gateway) {
//     if (m_working.ipv6Gateway == gateway) return;
//     m_working.ipv6Gateway = gateway;
//     emit ipv6GatewayChanged();
//     updateModificationFlag();
// }

// QStringList ConnectionEditorModel::ipv6Dns() const { return m_working.ipv6Dns; }
// void ConnectionEditorModel::setIpv6Dns(const QStringList &dns) {
//     if (m_working.ipv6Dns == dns) return;
//     m_working.ipv6Dns = dns;
//     emit ipv6DnsChanged();
//     updateModificationFlag();
// }

bool ConnectionEditorModel::isModified() const { return m_isModified; }
bool ConnectionEditorModel::isNew() const { return m_isNew; }

// ========== 私有：更新修改标志 ==========

void ConnectionEditorModel::updateModificationFlag()
{
    bool modified =
        (m_working.id != m_original.id)
        || (m_working.autoconnect != m_original.autoconnect)
        || (m_working.autoconnectPriority != m_original.autoconnectPriority)
        || (m_working.interfaceName != m_original.interfaceName)
        || (m_working.ssid != m_original.ssid)
        || (m_working.wirelessSecurity != m_original.wirelessSecurity)
        || (m_working.wirelessPassword != m_original.wirelessPassword)
        || (m_working.mtu != m_original.mtu)
        || (m_working.ipv4Method != m_original.ipv4Method)
        || (m_working.ipv4Address != m_original.ipv4Address)
        || (m_working.ipv4Gateway != m_original.ipv4Gateway)
        || (m_working.ipv4Dns != m_original.ipv4Dns);
        // || (m_working.ipv6Method != m_original.ipv6Method)
        // || (m_working.ipv6Address != m_original.ipv6Address)
        // || (m_working.ipv6Gateway != m_original.ipv6Gateway)
        // || (m_working.ipv6Dns != m_original.ipv6Dns);

    if (m_isModified != modified) {
        m_isModified = modified;
        emit isModifiedChanged(m_isModified);
    }
}

} // namespace Net
