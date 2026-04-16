import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1920
    height: 1080
    title: "Network Manager"

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: nm ? nm.deviceModel : null

            delegate: Rectangle {
                width: ListView.view.width
                height: 50
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 20

                    Text {
                        text: devName
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "Type: " + devType
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "MAC: " + Hwaddr
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "State: " + devState
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "StateReason: " + stateReason
                        Layout.fillWidth: true
                    }
		    
                    Text {
                        text: "IPv4: " + ipv4Add
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "IPv4GW: " + ipv4Gateway
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "IPv4 DNS: " + ipv4Dns
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "IPv6: " + ipv6Add
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "CurConnect: " + activeConnection
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "carrier: " + carrier
                        Layout.fillWidth: true
                    }
		    
                    Text {
                        text: "speed: " + speed
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "signal: " + _signal
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
