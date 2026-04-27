import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "pages"

ApplicationWindow {
    width: 1000
    height: 650
    visible: true
    title: "Network Manager"

    ColumnLayout {
        anchors.fill: parent

        // 顶栏
        TopBar {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 侧边栏
            SideBar {
                id: sideBar
                Layout.preferredWidth: 180
                Layout.minimumWidth: 100
                Layout.fillHeight: true
                onPageSelected: (index) => {
                        stackView.currentIndex = index
                    }
            }

            // 页面区域
            StackLayout {
                id: stackView

                onCurrentIndexChanged: {
                    nm.apService.setScanEnable(currentIndex === 1)
                }

                Layout.fillWidth: true
                Layout.fillHeight: true

                DevicePage {}
                WifiPage {}
                ConnectionsPage {}
            }
        }
    }
}