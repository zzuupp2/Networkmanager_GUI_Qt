import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Networkmanager_GUI_Qt

Rectangle {
    id: root
    height: 50
    color: "#494856"
    radius: 6

    readonly property color clGreen:  "#4caf50"
    readonly property color clOrange: "#ff9800"
    readonly property color clRed:    "#f44336"
    readonly property color clWhite:  "#ffffff"
    readonly property color clGray:   "#999999"

    // ===== 强制 primaryInfo 绑定量新求值的 counter =====
    property int primaryRefresh: 0

    readonly property var primaryInfo: {
        primaryRefresh  // 依赖追踪
        const row = nm.runtimeModel.primaryRow
        if (row < 0) return null
        return nm.runtimeModel.get(row)
    }

    Connections {
        target: nm.runtimeModel
        function onPrimaryRowChanged() { primaryRefresh++ }
        function onPrimaryChanged()  { primaryRefresh++ }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        // ===== 设备类型图标 =====
        Label {
            text: {
                if (!root.primaryInfo) return "🔌"
                return root.primaryInfo.deviceType === "wireless" ? "📶" : "🔌"
            }
            font.pixelSize: 16
            color: clWhite
        }

        // ===== 连接名称 =====
        Label {
            text: {
                if (!root.primaryInfo) return "未连接"
                const d = root.primaryInfo
                return d.ssid || d.interface || "已连接"
            }
            color: clWhite
            font.pixelSize: 13
            font.bold: true
            elide: Text.ElideRight
            Layout.maximumWidth: 180
        }

        // ===== 分隔点 =====
        Label {
            text: "·"
            color: clGray
            visible: !!root.primaryInfo
        }

        // ===== IP 地址（含掩码） =====
        Label {
            text: {
                if (!root.primaryInfo) return ""
                const ip = root.primaryInfo.ipv4
                return ip || "获取中..."
            }
            color: clWhite
            font.pixelSize: 12
            visible: !!root.primaryInfo
        }

        // ===== 分隔点 =====
        Label {
            text: "·"
            color: clGray
            visible: !!root.primaryInfo
        }

        // ===== 连接状态文字 =====
        Label {
            text: {
                if (!root.primaryInfo) return ""
                return root.primaryInfo.statusText || ""
            }
            color: {
                if (!root.primaryInfo) return clGray
                switch (root.primaryInfo.activeState) {
                    case 1: return clOrange   // Activating
                    case 2: return clGreen    // Activated
                    case 3: return clOrange   // Deactivating
                    default: return clGray
                }
            }
            font.pixelSize: 12
            visible: !!root.primaryInfo
        }

        // ===== 联网状态指示 =====
        Label {
            text: "·"
            color: clGray
            visible: !!root.primaryInfo && root.primaryInfo.activeState === 2
        }

        Rectangle {
            width: 8; height: 8
            radius: 4
            visible: !!root.primaryInfo && root.primaryInfo.activeState === 2
            color: {
                switch (nm.runtimeModel.connectivity) {
                    case 4: return clGreen   // Full
                    case 3: return clOrange  // Limited
                    case 2: return clOrange  // Portal
                    case 1: return clRed     // None
                    default: return clGray   // Unknown
                }
            }
        }

        Label {
            visible: !!root.primaryInfo && root.primaryInfo.activeState === 2
            text: {
                switch (nm.runtimeModel.connectivity) {
                    case 4: return "已联网"
                    case 3: return "受限"
                    case 2: return "需登录"
                    case 1: return "无网络"
                    default: return ""
                }
            }
            color: clWhite
            font.pixelSize: 12
        }

        Item { Layout.fillWidth: true }

        // ===== WiFi 开关 =====
        Switch {
            text: "WiFi"
            checked: nm.wirelessEnabled
            onToggled: nm.setWirelessEnabled(checked)
        }

        // ===== 飞行模式 =====
        Switch {
            text: "飞行模式"
            checked: !nm.networkingEnabled
            onToggled: nm.setNetworkingEnabled(!checked)
        }
    }
}
