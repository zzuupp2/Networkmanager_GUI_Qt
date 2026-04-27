// DeviceCard.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string devName: ""
    property string devType: ""
    property string devMac: ""
    property int devMtu: -1

    property string devState: ""
    property string stateReason: ""

    property string ipv4Add: ""
    property string ipv4Gateway: ""
    property string ipv4Dns: ""
    property string ipv6Add: ""
    property string curCon: ""

    property string wiredOrWireless: ""

    signal showReason(string msg)

    radius: 10
    border.color: "#dddddd"
    color: devState === "Activated" ? "#e8f5e9" : "#f5f5f5"

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

        // ===== 信息区域（两列结构）=====
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // ================= 左信息列 =================
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                // 名称 + 状态
                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: devName
                        font.bold: true
                        Layout.fillWidth: true
                        color: "#333"
                    }

                  //  Label {
                  //      text: devState
                  //      color: devState === "Activated" ? "green" : "#666"
                  //  }

                }

                // MAC
                Label {
                    visible: devMac !== ""
                    text: "MAC: " + devMac
                    font.pixelSize: 12
                    color: "#777"
                }

                Label {
                    visible: devMtu !== -1
                    text: "MTU: " + devMtu
                    font.pixelSize: 12
                    color: "#777"
                }

                // 当前连接
                Label {
                    visible: curCon !== ""
                    text: "连接: " + curCon
                    font.pixelSize: 12
                    color: "#777"
                }

                // 设备特有信息
                RowLayout {
                    spacing: 10

                    Label {
                        text: wiredOrWireless
                        font.pixelSize: 12
                        color: "#777"
                    }

                }
            }


            // ================= 右信息列（网络） =================
            ColumnLayout {
                Layout.preferredWidth: 200
                Layout.alignment: Qt.AlignTop
                spacing: 3

                Label {
                    visible: ipv4Add !== ""
                    text: "IPv4: " + ipv4Add
                    font.pixelSize: 12
                    color: "#555"
                }

                Label {
                    visible: ipv4Gateway !== ""
                    text: "GW: " + ipv4Gateway
                    font.pixelSize: 11
                    color: "#777"
                }

                Label {
                    visible: ipv4Dns !== ""
                    text: "DNS: " + ipv4Dns
                    font.pixelSize: 11
                    color: "#777"
                }
            }
        }

        Button {
            visible: stateReason !== ""
            text: "原因"
            onClicked: root.showReason(stateReason)
        }
        // ===== 右侧按钮 =====
        Button {
            Layout.alignment: Qt.AlignVCenter
            text: "添加连接"
            onClicked: console.log("TODO")
        }
    }
}