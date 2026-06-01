// DeviceCard.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string devName: ""
    property string devType: ""
    property string devHwAddr: ""
    property int devMtu: -1

    property string devState: ""
    property string devStateSummary: ""
    property int devStateEnum: 0  // NetworkManager::Device::State enum

    property string curConnection: ""

    signal showDetail()

    radius: 10
    border.color: "#dddddd"

    // 根据实际 NM Device::State 枚举值着色
    readonly property var stateColors: ({
        0:   "#f5f5f5",  // Unknown
        10:  "#ffe0b2",  // Unmanaged
        20:  "#ffcdd2",  // Unavailable
        30:  "#bbdefb",  // Disconnected
        40:  "#fff9c4",  // Preparing
        50:  "#fff9c4",  // ConfiguringHardware
        60:  "#fff9c4",  // NeedAuth
        70:  "#fff9c4",  // ConfiguringIp
        80:  "#fff9c4",  // CheckingIp
        90:  "#fff9c4",  // WaitingForSecondaries
        100: "#c8e6c9",  // Activated
        110: "#ffe0b2",  // Deactivating
        120: "#ffcdd2",  // Failed
    })

    color: stateColors[devStateEnum] || "#f5f5f5"

    implicitHeight: content.implicitHeight + 16

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 10
        spacing: 14

        // ===== 左侧图标 =====
        Rectangle {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 42
            radius: 8

            color: {
                if (devType === "WiFi") return "#64b5f6"
                if (devType === "Ethernet") return "#81c784"
                return "#bdbdbd"
            }

            Label {
                anchors.centerIn: parent
                text: devType.length > 0 ? devType[0] : "?"
                color: "white"
                font.bold: true
            }
        }

        // ===== 信息区域 =====
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            // 接口名 + 状态标签
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: devName
                    font.bold: true
                    Layout.fillWidth: true
                    color: "#333"
                }

                Rectangle {
                    radius: 4
                    readonly property var badgeColors: ({
                        0:   "#9e9e9e",  // Unknown — 灰
                        10:  "#ff9800",  // Unmanaged — 橙
                        20:  "#f44336",  // Unavailable — 红
                        30:  "#2196f3",  // Disconnected — 蓝
                        40:  "#ff9800",  // Preparing — 橙
                        50:  "#ff9800",  // ConfiguringHardware — 橙
                        60:  "#ff9800",  // NeedAuth — 橙
                        70:  "#ff9800",  // ConfiguringIp — 橙
                        80:  "#ff9800",  // CheckingIp — 橙
                        90:  "#ff9800",  // WaitingForSecondaries — 橙
                        100: "#4caf50",  // Activated — 绿
                        110: "#ff9800",  // Deactivating — 橙
                        120: "#f44336",  // Failed — 红
                    })
                    color: badgeColors[devStateEnum] || "#9e9e9e"
                    height: 22
                    implicitWidth: stateLabel.implicitWidth + 12

                    Label {
                        id: stateLabel
                        anchors.centerIn: parent
                        text: devStateSummary || devState
                        color: "white"
                        font.pixelSize: 11
                        font.bold: true
                    }
                }
            }

            // MAC 地址
            Label {
                visible: devHwAddr !== ""
                text: "MAC: " + devHwAddr
                font.pixelSize: 12
                color: "#777"
            }

            // 当前连接
            Label {
                visible: curConnection !== ""
                text: "连接: " + curConnection
                font.pixelSize: 12
                color: "#777"
            }
        }

        // ===== 详情按钮 =====
        Button {
            Layout.alignment: Qt.AlignVCenter
            text: "详情"
            onClicked: root.showDetail()
        }
    }
}
