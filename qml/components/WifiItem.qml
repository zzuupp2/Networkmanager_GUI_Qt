import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string ssid: ""
    property int strength: 0
    property bool security: false
    property bool connected: false

    signal connectClicked()

    radius: 10
    border.color: "#dddddd"

    color: connected ? "#e8f5e9" : "#ffffff"

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
            color: "#64b5f6"

            Label {
                anchors.centerIn: parent
                text: "📶"
                font.pixelSize: 18
            }
        }

        // ===== 信息区域 =====
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            // SSID + 状态
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
                }
            }

            // 信号强度
            Label {
                text: "信号强度: " + strength + "%"
                font.pixelSize: 12
                color: "#666"
            }
        }

        // ===== 右侧信息 =====
        RowLayout {
            spacing: 10

            // 信号图标（简化版）
            Label {
                text: strength > 75 ? "████"
                     : strength > 50 ? "███"
                     : strength > 25 ? "██"
                     : "█"
            }

            // 加密
            Label {
                text: security ? "🔒" : ""
            }
        }

        // ===== 操作按钮 =====
        Button {
            Layout.alignment: Qt.AlignVCenter

            text: connected ? "已连接" : "连接"
            enabled: !connected

            onClicked: root.connectClicked()
        }
    }
}