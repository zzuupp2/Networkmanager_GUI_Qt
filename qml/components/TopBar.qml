import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Networkmanager_GUI_Qt

Rectangle {
    height: 50
    color: "#2c3e50"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: "📶 MyWiFi | 192.168.1.10"
            color: "white"
        }

        Item { Layout.fillWidth: true }

        Switch { text: "WiFi" }
        Switch { text: "飞行模式" }
    }
}