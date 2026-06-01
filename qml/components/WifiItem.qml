import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string ssid: ""
    property int strength: 0
    property bool security: false
    property bool connected: false
    property string band: ""
    property string bssid: ""
    property string securityType: ""
    property int bandwidth: 0
    property string activeConnectionUuid: ""
    property string savedConnectionUuid: ""

    signal connectClicked()

    radius: 10
    color: connected ? "#52A08D" : "#C1CEDA"
    implicitHeight: content.implicitHeight + 16

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        // ===== 左侧图标 =====
        Rectangle {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 42
            radius: 8
            color: connected ? "#66bb6a" : "#64b5f6"

            Label {
                anchors.centerIn: parent
                text: connected ? "📶" : "📡"
                font.pixelSize: 18
            }
        }

        // ===== 信息区域 =====
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            // Row 1: SSID + 已连接标记
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: ssid
                    font.bold: true
                    Layout.fillWidth: true
                    color: "#333"
                }

                Label {
                    text: connected ? "已连接" : ""
                    color: "green"
                    font.pixelSize: 12
                }
            }

            // Row 2: 安全类型 · 频段 · 带宽
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    text: securityType
                    color: securityType === "开放" ? "#e65100"
                           : securityType === "WEP" ? "#d32f2f"
                           : "#2e7d32"
                    font.pixelSize: 12
                }

                Rectangle {
                    width: 1
                    height: 12
                    color: "#ccc"
                    visible: !!band
                }

                Label {
                    text: band
                    color: "#666"
                    font.pixelSize: 12
                }

                Rectangle {
                    width: 1
                    height: 12
                    color: "#ccc"
                    visible: bandwidth > 0
                }

                Label {
                    text: bandwidth > 0 ? bandwidth + " MHz" : ""
                    color: "#666"
                    font.pixelSize: 12
                }
            }

            // Row 3: 信号强度 · BSSID
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    text: "信号: " + strength + "%"
                    color: strength > 50 ? "#2e7d32"
                           : strength > 25 ? "#f57c00"
                           : "#d32f2f"
                    font.pixelSize: 12
                }

                Label {
                    text: strength > 75 ? "████"
                         : strength > 50 ? "███"
                         : strength > 25 ? "██"
                         : "█"
                    color: "#888"
                    font.pixelSize: 10
                }

                Rectangle {
                    width: 1
                    height: 12
                    color: "#ccc"
                    visible: !!bssid
                }

                Label {
                    text: bssid ? "BSSID: " + bssid : ""
                    color: connected ? "#e3e2e2" : "#999"
                    font.pixelSize: 11
                }
            }
        }

        // ===== 右侧按钮 =====
        Button {
            Layout.alignment: Qt.AlignVCenter
            text: connected ? "断开" : "连接"
            enabled: connected ? !!activeConnectionUuid : !!ssid
            onClicked: {
                if (connected)
                    nm.manager.deactivateConnection(activeConnectionUuid)
                else if (savedConnectionUuid)
                    nm.manager.activateConnection(savedConnectionUuid)
                else
                    root.connectClicked()
            }
        }
    }
}
