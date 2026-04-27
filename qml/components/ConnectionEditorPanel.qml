import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property var editor
    property string currentUuid: editor.uuid

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // =========================
        // 顶部切换菜单
        // =========================
        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton { text: "状态" }
            TabButton { text: "配置" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // =========================
            // ① 状态页（只读）
            // =========================
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Label {
                        text: "连接状态"
                        font.bold: true
                    }

                    // ===== 当前连接名 =====
                    Label {
                        text: "名称: " + editor.id
                    }

                    // ===== 状态 =====
                    Label {
                        text: "状态: " + (nm.isActive(currentUuid) ? "已连接" : "未连接")
                        color: nm.isActive(currentUuid) ? "green" : "#666"
                    }

                    // ===== IP 信息（来自 DeviceModel）=====
                    Label {
                        text: "IPv4: " + nm.getIpv4(currentUuid)
                    }

                    Label {
                        text: "网关: " + nm.getGateway(currentUuid)
                    }

                    Label {
                        text: "DNS: " + nm.getDns(currentUuid)
                        wrapMode: Text.Wrap
                    }

                    Item { Layout.fillHeight: true }

                    // ===== 操作按钮 =====
                    RowLayout {
                        Button {
                            text: "连接"
                            enabled: !nm.isActive(currentUuid)
                            onClicked: nm.manager.activate(currentUuid)
                        }

                        Button {
                            text: "断开"
                            enabled: nm.isActive(currentUuid)
                            onClicked: nm.manager.deactivate(currentUuid)
                        }
                    }
                }
            }

            // =========================
            // ② 配置页（可编辑）
            // =========================
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Label {
                        text: editor.isNew ? "新建连接" : "编辑配置"
                        font.bold: true
                    }

                    // ===== 基本 =====
                    GroupBox {
                        title: "基本信息"
                        Layout.fillWidth: true

                        ColumnLayout {

                            TextField {
                                Layout.fillWidth: true
                                text: editor.id
                                placeholderText: "名称"
                                onTextChanged: editor.id = text
                            }

                            CheckBox {
                                text: "自动连接"
                                checked: editor.autoconnect
                                onToggled: editor.autoconnect = checked
                            }

                            TextField {
                                Layout.fillWidth: true
                                text: editor.interfaceName
                                placeholderText: "接口"
                                onTextChanged: editor.interfaceName = text
                            }
                        }
                    }

                    // ===== WiFi =====
                    GroupBox {
                        title: "无线设置"
                        visible: editor.type === "802-11-wireless"

                        ColumnLayout {
                            TextField {
                                text: editor.ssid
                                placeholderText: "SSID"
                                onTextChanged: editor.ssid = text
                            }

                            TextField {
                                text: editor.wirelessPassword
                                echoMode: TextInput.Password
                                placeholderText: "密码"
                                onTextChanged: editor.wirelessPassword = text
                            }
                        }
                    }

                    // ===== IPv4 =====
                    GroupBox {
                        title: "IPv4"
                        Layout.fillWidth: true

                        ColumnLayout {

                            ComboBox {
                                model: ["auto", "manual"]
                                currentIndex: editor.ipv4Method === "manual" ? 1 : 0

                                onCurrentIndexChanged: {
                                    editor.ipv4Method = currentIndex === 1 ? "manual" : "auto"
                                }
                            }

                            TextField {
                                visible: editor.ipv4Method === "manual"
                                text: editor.ipv4Address
                                placeholderText: "IP"
                                onTextChanged: editor.ipv4Address = text
                            }

                            TextField {
                                visible: editor.ipv4Method === "manual"
                                text: editor.ipv4Gateway
                                placeholderText: "网关"
                                onTextChanged: editor.ipv4Gateway = text
                            }

                            TextField {
                                text: editor.ipv4Dns.join(",")
                                placeholderText: "DNS"
                                onTextChanged: editor.ipv4Dns = text.split(",")
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }

                    // ===== 操作 =====
                    RowLayout {
                        Layout.alignment: Qt.AlignRight

                        Button {
                            text: "取消"
                            onClicked: editor.reset()
                        }

                        Button {
                            text: "保存"
                            enabled: editor.isModified
                            onClicked: {
                                nm.manager.apply(editor.toSettingsMap(), editor.isNew)
                            }
                        }
                    }
                }
            }
        }
    }
}