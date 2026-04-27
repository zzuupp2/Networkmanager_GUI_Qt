// DevicePage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {
    // ===== 弹窗（去掉 implicitWidth 循环）=====
    Popup {
        id: reasonDialog
        modal: true
        focus: true
        width: 300

        property string message: ""

        background: Rectangle {
            radius: 8
            color: "white"
            border.color: "#ccc"
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            Label {
                text: "状态变更原因"
                font.bold: true
            }

            Label {
                text: reasonDialog.message
                wrapMode: Text.WordWrap
            }

            Button {
                text: "关闭"
                onClicked: reasonDialog.close()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // ===== 过滤栏 =====
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "设备过滤："
                font.bold: true
            }

            ComboBox {
                id: filterBox
                model: ["仅主设备", "主+副设备", "全部设备"]

                onCurrentIndexChanged: {
                    nm.deviceProxy.setMode(currentIndex)
                }
            }
        }

        // ===== 设备列表 =====
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: nm.deviceProxy
            spacing: 8
            clip: true

            delegate: DeviceCard {
                width: listView.width

                devName: model.devName || ""
                devType: model.devType || ""
                devMac: model.devMac || ""
                devMtu: model.devMtu || -1

                devState: model.devState || ""
                stateReason: model.stateReason || ""

                ipv4Add: model.ipv4Add || ""
                ipv4Gateway: model.ipv4Gateway || ""
                ipv4Dns: model.ipv4Dns ? model.ipv4Dns.join(", ") : ""
                ipv6Add: model.ipv6Add || ""
                curCon: model.curConnection || ""

                wiredOrWireless: model.wiredOrWireless || ""

                onShowReason: (msg) => {
                    reasonDialog.message = msg
                    reasonDialog.open()
                }
            }

            footer: Item {
                width: parent.width
                height: listView.count === 0 ? 120 : 0

                Label {
                    anchors.centerIn: parent
                    visible: listView.count === 0
                    text: "没有设备"
                    color: "#888"
                }
            }
        }
    }
}
