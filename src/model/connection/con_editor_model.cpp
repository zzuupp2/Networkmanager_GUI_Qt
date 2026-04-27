#include <QDebug>

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
    m_original = info;
    m_working = info;
    m_isNew = false;
    m_isModified = false;

    // 通知所有属性变更，触发 QML 界面完全刷新
    emit idChanged();
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

    m_isNew = true;
    loadFromSettingInfo(info);
}

// ========== 操作 ==========

void ConnectionEditorModel::reset()
{
    m_working = m_original;
    m_isModified = false;

    // 刷新所有属性到原始值
    emit idChanged();
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

ConnectionSettingInfo ConnectionEditorModel::toSettingInfo() const
{
    return m_working;
}

QVariantMap ConnectionEditorModel::toSettingsMap() const
{
    QVariantMap topMap;

    // connection 子映射
    QVariantMap conn;
    conn["id"] = m_working.id;
    if (!m_working.uuid.isEmpty())
        conn["uuid"] = m_working.uuid;
    conn["type"] = m_working.type;
    conn["autoconnect"] = m_working.autoconnect;
    conn["autoconnect-priority"] = m_working.autoconnectPriority;
    if (!m_working.interfaceName.isEmpty())
        conn["interface-name"] = m_working.interfaceName;
    topMap["connection"] = conn;

    // 802-11-wireless 子映射（仅当类型为无线）
    if (m_working.type == "802-11-wireless") {
        QVariantMap wifi;
        if (!m_working.ssid.isEmpty())
            wifi["ssid"] = m_working.ssid.toUtf8();   // NM 要求字节数组
        // 安全设置
        if (!m_working.wirelessSecurity.isEmpty() && m_working.wirelessSecurity != "none") {
            QVariantMap sec;
            sec["key-mgmt"] = m_working.wirelessSecurity;
            if (m_working.wirelessSecurity == "wpa-psk" && !m_working.wirelessPassword.isEmpty())
                sec["psk"] = m_working.wirelessPassword;
            // 可扩展其他安全类型
            wifi["security"] = sec;
        }
        topMap["802-11-wireless"] = wifi;
    }

    // 802-3-ethernet 子映射（有线）
    if (m_working.type == "802-3-ethernet") {
        QVariantMap eth;
        if (m_working.mtu > 0)
            eth["mtu"] = m_working.mtu;
        topMap["802-3-ethernet"] = eth;
    }

    // ipv4 子映射
    QVariantMap ipv4;
    ipv4["method"] = m_working.ipv4Method;
    if (m_working.ipv4Method == "manual") {
        // 地址处理
        if (!m_working.ipv4Address.isEmpty()) {
            QVariantMap addrEntry;
            addrEntry["address"] = m_working.ipv4Address;
            addrEntry["prefix"] = 24;   // 简化，可扩展解析
            QVariantList addrs;
            addrs.append(addrEntry);
            ipv4["addresses"] = addrs;
        }
        if (!m_working.ipv4Gateway.isEmpty())
            ipv4["gateway"] = m_working.ipv4Gateway;
        if (!m_working.ipv4Dns.isEmpty()) {
            QVariantList dns;
            for (const auto &d : m_working.ipv4Dns)
                dns.append(d);
            ipv4["dns"] = dns;
        }
    }
    topMap["ipv4"] = ipv4;

    // // ipv6 子映射
    // QVariantMap ipv6;
    // ipv6["method"] = m_working.ipv6Method;
    // if (m_working.ipv6Method == "manual") {
    //     if (!m_working.ipv6Address.isEmpty()) {
    //         QVariantMap addr;
    //         addr["address"] = m_working.ipv6Address;
    //         addr["prefix"] = 64;
    //         QVariantList addrs;
    //         addrs.append(addr);
    //         ipv6["addresses"] = addrs;
    //     }
    //     if (!m_working.ipv6Gateway.isEmpty())
    //         ipv6["gateway"] = m_working.ipv6Gateway;
    //     if (!m_working.ipv6Dns.isEmpty()) {
    //         QVariantList dns;
    //         for (const auto &d : m_working.ipv6Dns)
    //             dns.append(d);
    //         ipv6["dns"] = dns;
    //     }
    // }
    // topMap["ipv6"] = ipv6;

    // 合并 extra 字段（如果存在）
    for (auto it = m_working.extra.begin(); it != m_working.extra.end(); ++it) {
        topMap[it.key()] = it.value();
    }

    return topMap;
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