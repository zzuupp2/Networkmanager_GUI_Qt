import QtQuick
import QtQuick.Controls

Item {
    signal pageSelected(int index)

    property int currentIndex: 0

    Rectangle {
        color: "#494856"
        anchors.fill: parent
        radius: 6

        ListView {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 6
            model: ["设备", "WiFi", "已保存网络"]

            delegate: ItemDelegate {
                width: parent.width
                height: 44

                background: Rectangle {
                    radius: 6
                    color: {
                        if (currentIndex === index) return "#39d7b0"
                        if (hovered) return "#52A08D"
                        return "transparent"
                    }
                }

                contentItem: Text {
                    text: modelData
                    color: currentIndex === index ? "#ffffff" : "#b0bec5"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 15
                    font.bold: currentIndex === index
                }

                onClicked: {
                    currentIndex = index
                    pageSelected(index)
                }
            }
        }
    }
}
