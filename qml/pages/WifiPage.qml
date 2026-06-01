import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {
    // ===== 三级灰度色阶 =====
    readonly property color clPrimary:   "#1a1a1a"
    readonly property color clSecondary: "#666666"
    readonly property color clDisabled:  "#999999"

    property string pendingSsid: ""

    function openConnectDialog(ssid) {
        pendingSsid = ssid
        connectDialog.open()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // ===== 工具栏 =====
        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "扫描"
                onClicked: nm.apService.requestScan(false)
            }
         }

        // ===== WiFi 列表 =====
        ListView {
            id: wifiList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true

            model: nm.apModel

            delegate: WifiItem {
                width: wifiList.width

                ssid: model.ssid || ""
                strength: model.strength || 0
                security: model.security || false
                connected: model.connected || false
                band: model.band || ""
                bssid: model.bssid || ""
                securityType: model.securityType || ""
                bandwidth: model.bandwidth || 0
                activeConnectionUuid: model.activeConnectionUuid || ""
                savedConnectionUuid: model.savedConnectionUuid || ""

                onConnectClicked:
                    openConnectDialog(model.ssid)
            }

            footer: Item {
                width: parent.width
                height: wifiList.count === 0 ? 120 : 0

                Label {
                    anchors.centerIn: parent
                    visible: wifiList.count === 0
                    text: "没有可用 WiFi"
                    color: clDisabled
                }
            }
        }
    }

    // =================================
    // WiFi 连接对话框
    // =================================

    Dialog {
        id: connectDialog
        modal: true
        width: 360
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        header: Label {
            text: "连接 WiFi"
            font.bold: true
            font.pixelSize: 14
            color: clPrimary
            padding: 16
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                Label {
                    text: "SSID"
                    color: clSecondary
                    Layout.preferredWidth: 60
                }

                Label {
                    Layout.fillWidth: true
                    text: pendingSsid
                    font.bold: true
                    color: "#C1CEDA"
                }

                Label {
                    text: "密码"
                    color: clSecondary
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    TextField {
                        id: pwdField
                        Layout.fillWidth: true
                        echoMode: pwdToggle.checked ? TextInput.Normal : TextInput.Password
                        color: "#C1CEDA"
                    }

                    Button {
                        id: pwdToggle
                        checkable: true
                        text: checked ? "隐藏" : "显示"
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                height: 4
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10

                Button {
                    text: "取消"
                    onClicked: connectDialog.close()
                }

                Button {
                    text: "连接"
                    onClicked: {
                        nm.manager.createAndActivateWifiConnection(
                            pendingSsid,
                            pwdField.text
                        )
                        connectDialog.close()
                        pwdField.text = ""
                    }
                }
            }
        }
    }
}
