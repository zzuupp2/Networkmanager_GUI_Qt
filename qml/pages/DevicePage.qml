// DevicePage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {
    // 三级灰度色阶
    readonly property color clPrimary:   "#1a1a1a"
    readonly property color clSecondary: "#666666"
    readonly property color clDisabled:  "#999999"

    // ===== 设备详情弹窗 =====
    Dialog {
        id: detailDialog
        modal: true
        width: 400
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "#C1CEDA"
            radius: 8
            border.color: "#A0B4C8"
        }

        header: Label {
            text: "设备详情 — " + (detailDialog.detailName || "")
            font.bold: true
            font.pixelSize: 14
            color: clPrimary
            padding: 16
            background: Rectangle {
                color: "#C1CEDA"
                radius: 8
            }
        }

        property string detailName: ""
        property string detailDriver: ""
        property string detailDriverVer: ""
        property string detailFirmware: ""
        property string detailStateSummary: ""

        property int detailMtu: -1

        // 有线
        property int detailBitRate: 0
        property bool detailCarrier: false

        // 无线
        property string detailSsid: ""
        property int detailSignal: 0

        function formatSpeed(kbps) {
            if (kbps <= 0) return ""
            var mbps = kbps / 1000.0
            if (mbps >= 1000)
                return (mbps / 1000.0).toFixed(1) + " Gbps"
            return Math.round(mbps) + " Mbps"
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            // ----- 状态总览 -----
            Label {
                visible: detailDialog.detailStateSummary !== ""
                text: detailDialog.detailStateSummary
                font.bold: true
                font.pixelSize: 16
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: {
                    var s = detailDialog.detailStateSummary
                    if (s.indexOf("已连接") === 0) return "#2e7d32"
                    if (s.indexOf("连接失败") >= 0) return "#c62828"
                    if (s.indexOf("失败") === 0) return "#c62828"
                    if (s.indexOf("不可用") >= 0) return "#c62828"
                    if (s.indexOf("正在断开") === 0 || s.indexOf("断开") >= 0) return "#e65100"
                    if (s.indexOf("正在") === 0) return "#e65100"
                    if (s.indexOf("未连接") === 0 || s.indexOf("未管理") === 0) return "#1565c0"
                    if (s.indexOf("未知") === 0) return clDisabled
                    return clPrimary
                }
            }

            Rectangle {
                visible: detailDialog.detailStateSummary !== ""
                Layout.fillWidth: true
                height: 1
                color: "#494856"
            }

            // ----- 驱动信息 -----
            Label {
                text: "驱动信息"
                font.bold: true
                font.pixelSize: 13
                color: clPrimary
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#494856"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label { text: "驱动"; color: clSecondary }
                Label { text: detailDialog.detailDriver || "—"; color: clPrimary; font.bold: true }

                Label { text: "版本"; color: clSecondary }
                Label { text: detailDialog.detailDriverVer || "—"; color: clPrimary; font.bold: true }

                Label { text: "固件"; color: clSecondary }
                Label { text: detailDialog.detailFirmware || "—"; color: clPrimary; font.bold: true }


            }

            // ----- 链路 -----
            Label {
                text: "链路"
                font.bold: true
                font.pixelSize: 13
                color: clPrimary
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#494856"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label { text: "MTU"; color: clSecondary }
                Label { text: detailDialog.detailMtu > 0 ? detailDialog.detailMtu : "—"; color: clPrimary; font.bold: true }

                Label { visible: detailDialog.detailBitRate > 0; text: "速率"; color: clSecondary }
                Label { visible: detailDialog.detailBitRate > 0; text: detailDialog.formatSpeed(detailDialog.detailBitRate); color: clPrimary; font.bold: true }

                Label { visible: detailDialog.detailCarrier; text: "链路状态"; color: clSecondary }
                Label { visible: detailDialog.detailCarrier; text: detailDialog.detailCarrier ? "已连接" : "未连接"; color: clPrimary; font.bold: true }
            }

            // ----- 无线专有 -----
            Label {
                visible: detailDialog.detailSsid !== ""
                text: "无线"
                font.bold: true
                font.pixelSize: 13
                color: clPrimary
            }

            Rectangle {
                visible: detailDialog.detailSsid !== ""
                Layout.fillWidth: true
                height: 1
                color: "#494856"
            }

            GridLayout {
                visible: detailDialog.detailSsid !== ""
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label { text: "SSID"; color: clSecondary }
                Label { text: detailDialog.detailSsid; color: clPrimary; font.bold: true }

                Label { text: "信号"; color: clSecondary }
                Label { text: detailDialog.detailSignal + "%"; color: clPrimary; font.bold: true }
            }

            // ----- 关闭按钮 -----
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                Button {
                    text: "关闭"
                    onClicked: detailDialog.close()
                }
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
                devHwAddr: model.devHwAddr || ""
                devState: model.devState || ""
                devStateSummary: model.devStateSummary || ""
                devStateEnum: model.devStateEnum ?? 0
                curConnection: model.curConnection || ""

                onShowDetail: {
                    detailDialog.detailName = model.devName || ""
                    detailDialog.detailDriver = model.devDriver || ""
                    detailDialog.detailDriverVer = model.devDriverVersion || ""
                    detailDialog.detailFirmware = model.devFirmwareVersion || ""
                    detailDialog.detailStateSummary = model.devStateSummary || ""

                    detailDialog.detailMtu = model.devMtu || -1
                    detailDialog.detailBitRate = model.devBitRate || 0
                    detailDialog.detailCarrier = model.devCarrier || false
                    detailDialog.detailSsid = model.devActiveSsid || ""
                    detailDialog.detailSignal = model.devSignalStrength || 0
                    detailDialog.open()
                }
            }

            footer: Item {
                width: parent.width
                height: listView.count === 0 ? 120 : 0

                Label {
                    anchors.centerIn: parent
                    visible: listView.count === 0
                    text: "没有设备"
                    color: clDisabled
                }
            }
        }
    }
}
