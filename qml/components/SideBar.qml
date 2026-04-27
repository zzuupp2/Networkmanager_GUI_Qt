import QtQuick
import QtQuick.Controls

Item {

    signal pageSelected(int index)

    Rectangle {
        color: "#2c3e50"
        anchors.fill: parent

        ListView {
            anchors.fill: parent
            anchors.margins: 5   // 留一点边距更好看
            spacing: 10
            model: ["设备", "WiFi", "已保存网络"]

            delegate: ItemDelegate {
                width: parent.width
                text: modelData

                // 👉 让文字在深色背景上可见
                contentItem: Text {
                    text: modelData
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked:() => {
                    pageSelected(index)
                }
            }
        }
    }
}