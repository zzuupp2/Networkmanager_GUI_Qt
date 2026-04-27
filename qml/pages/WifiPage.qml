import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {

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

            model: nm.apModel   // ⭐ 关键

            delegate: WifiItem {
                width: wifiList.width

                ssid: model.ssid || ""
                strength: model.strength || 0
                security: model.security || false
                connected: model.connected || false

                onConnectClicked: {
                    nm.connectTo(ssid)
                }
            }

            // 空列表提示
            footer: Item {
                width: parent.width
                height: wifiList.count === 0 ? 120 : 0

                Label {
                    anchors.centerIn: parent
                    visible: wifiList.count === 0
                    text: "没有可用 WiFi"
                    color: "#888"
                }
            }
        }
    }
}