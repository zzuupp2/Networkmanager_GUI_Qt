import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var editor
    required property var manager

    signal saved()
    signal cancelled()

    // ===== 三级灰度色阶 =====
    readonly property color clPrimary:   "#1a1a1a"
    readonly property color clSecondary: "#666666"
    readonly property color clDisabled:  "#999999"

    // ===== 辅助函数：根据值是否为空追加"（未配置）" =====
    function hint(base, val) {
        return val ? base : base + "（未配置）"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 10

            // =====================================
            // 基本信息
            // =====================================

            Label {
                text: "基本设置"
                font.bold: true
                font.pixelSize: 14
                color: root.clPrimary
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                Label {
                    text: "名称"
                    color: root.clSecondary
                    Layout.preferredWidth: 120
                }

                TextField {
                    Layout.fillWidth: true
                    text: root.editor.id
                    placeholderText: root.editor.id ? "" : "未配置"
                    placeholderTextColor: root.clDisabled
                    onEditingFinished:
                        root.editor.id = text
                }

                Label {
                    text: "类型"
                    color: root.clSecondary
                }

                TextField {
                    Layout.fillWidth: true
                    text: root.editor.type
                    readOnly: true
                    color: root.clDisabled
                }

                Label {
                    text: "接口"
                    color: root.clSecondary
                }

                ComboBox {
                    id: ifaceCombo
                    Layout.fillWidth: true
                    editable: false
                    model: nm.interfacesForConType(root.editor.type)

                    Component.onCompleted: {
                        const idx = find(root.editor.interfaceName)
                        if (idx !== -1) currentIndex = idx
                    }

                    Connections {
                        target: root.editor
                        function onInterfaceNameChanged() {
                            const idx = ifaceCombo.find(root.editor.interfaceName)
                            if (idx !== -1) ifaceCombo.currentIndex = idx
                        }
                    }

                    onActivated: root.editor.interfaceName = currentText

                    contentItem: Label {
                        leftPadding: 8
                        text: ifaceCombo.currentText || "留空自动选择"
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                height: 4
            }

            // =====================================
            // 自动连接
            // =====================================

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                CheckBox {
                    id: autoconnectCheck
                    checked: root.editor.autoconnect
                    onToggled:
                        root.editor.autoconnect = checked
                    contentItem: Label {
                        text: "自动连接，优先级为"
                        color: root.clSecondary
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + parent.spacing
                    }
                }

                SpinBox {
                    id: prioritySpin
                    Layout.fillWidth: true
                    from: -999
                    to: 999
                    value: root.editor.autoconnectPriority
                    enabled: autoconnectCheck.checked
                    onValueChanged:
                        root.editor.autoconnectPriority = value
                }
            }

            // =====================================
            // Wi-Fi 配置
            // =====================================

            Label {
                visible: root.editor.type === "802-11-wireless"
                text: "Wi-Fi 设置"
                font.bold: true
                font.pixelSize: 14
                color: root.clPrimary
            }

            Rectangle {
                visible: root.editor.type === "802-11-wireless"
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            GridLayout {
                visible: root.editor.type === "802-11-wireless"
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                Label {
                    text: hint("SSID", root.editor.ssid)
                    color: root.editor.ssid ? root.clSecondary : root.clDisabled
                }

                ComboBox {
                    id: ssidCombo
                    Layout.fillWidth: true
                    editable: true
                    model: nm.apModel
                    textRole: "ssid"
                    valueRole: "security"

                    Component.onCompleted:
                        editText = root.editor.ssid || ""

                    Connections {
                        target: root.editor
                        function onSsidChanged() {
                            if (ssidCombo.editText !== root.editor.ssid)
                                ssidCombo.editText = root.editor.ssid || ""
                        }
                    }

                    popup.clip: true
                    popup.contentItem.ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                    popup.height: Math.min(Math.max(ssidCombo.count * 36 + 10, 100), 300)
                    popup.onOpened:
                        nm.apService.requestScan(true)

                    onAccepted:
                        root.editor.ssid = editText

                    onActivated:
                        root.editor.ssid = currentText

                    contentItem: TextField {
                        leftPadding: 28
                        text: ssidCombo.editText
                        onTextChanged: ssidCombo.editText = text
                        font: ssidCombo.font
                        readOnly: !ssidCombo.editable
                        selectByMouse: true
                    }

                    Label {
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        z: 2
                        color: root.clSecondary
                        font.pixelSize: 14
                        text: {
                            const text = ssidCombo.editText
                            if (!text) return ""
                            const m = ssidCombo.model
                            for (let i = 0; i < ssidCombo.count; ++i) {
                                const idx = m.index(i, 0)
                                // 257 = SsidRole, 259 = SecurityRole
                                if (m.data(idx, 257) === text)
                                    return m.data(idx, 259) ? "🔒" : ""
                            }
                            return ""
                        }
                    }

                    delegate: ItemDelegate {
                        width: ListView.view.width
                        contentItem: RowLayout {
                            Label {
                                text: model.security ? "🔒" : ""
                                color: root.clSecondary
                                font.pixelSize: 12
                                Layout.minimumWidth: 16
                            }
                            Label {
                                text: model.ssid || ""
                                //color: root.clPrimary
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: model.band || ""
                                color: root.clSecondary
                                font.pixelSize: 12
                                Layout.minimumWidth: 48
                            }
                            Label {
                                text: model.strength ? model.strength + "%" : ""
                                color: root.clSecondary
                                font.pixelSize: 12
                                Layout.minimumWidth: 36
                            }
                        }
                    }
                }

                Label {
                    text: "安全类型"
                    color: root.clSecondary
                }

                ComboBox {
                    Layout.fillWidth: true
                    model: [
                        { text: "无", value: "" },
                        { text: "WPA2-PSK (wpa-psk)", value: "wpa-psk" },
                        { text: "WPA3-SAE (sae)", value: "sae" },
                        { text: "WPA-Enterprise (wpa-eap)", value: "wpa-eap" },
                        { text: "WEP", value: "wep" },
                        { text: "OWE", value: "owe" },
                        { text: "WPA-None", value: "wpa-none" },
                        { text: "IEEE 802.1X", value: "ieee8021x" }
                    ]
                    textRole: "text"
                    valueRole: "value"

                    currentIndex: {
                        const sec = root.editor.wirelessSecurity || ""
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].value === sec)
                                return i
                        }
                        return 0
                    }

                    onActivated:
                        root.editor.wirelessSecurity =
                            model[currentIndex].value
                }

                Label {
                    text: hint("密码", root.editor.wirelessPassword)
                    color: root.editor.wirelessPassword ? root.clSecondary : root.clDisabled
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    TextField {
                        id: pwdField
                        Layout.fillWidth: true
                        text: root.editor.wirelessPassword
                        placeholderText: root.editor.wirelessPassword ? "" : "未配置"
                        placeholderTextColor: root.clDisabled
                        echoMode: pwdToggle.checked ? TextInput.Normal : TextInput.Password
                        onEditingFinished:
                            root.editor.wirelessPassword = text
                    }

                    Button {
                        id: pwdToggle
                        checkable: true
                        text: checked ? "隐藏" : "显示"
                    }
                }
            }

            // =====================================
            // 有线配置
            // =====================================

            Label {
                visible: root.editor.type === "802-3-ethernet"
                text: "有线设置"
                font.bold: true
                font.pixelSize: 14
                color: root.clPrimary
            }

            Rectangle {
                visible: root.editor.type === "802-3-ethernet"
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            GridLayout {
                visible: root.editor.type === "802-3-ethernet"
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                // ----- MTU -----
                Label {
                    text: hint("MTU", root.editor.mtu > 0)
                    color: root.editor.mtu > 0 ? root.clSecondary : root.clDisabled
                }

                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 9000
                    value: root.editor.mtu
                    onValueChanged:
                        root.editor.mtu = value
                }

                // ----- 连接协商 -----
                Label {
                    text: "连接协商"
                    color: root.clSecondary
                }

                ComboBox {
                    id: autoNegCombo
                    Layout.fillWidth: true
                    model: [
                        { text: "自动", value: 1 },
                        { text: "手动", value: 0 },
                        { text: "忽略", value: -1 }
                    ]
                    textRole: "text"
                    valueRole: "value"

                    currentIndex: {
                        const v = root.editor.wiredAutoNegotiate
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].value === v)
                                return i
                        }
                        return 0
                    }

                    onActivated:
                        root.editor.wiredAutoNegotiate =
                            model[currentIndex].value
                }

                // ----- 速率（仅手动模式可编辑）-----
                Label {
                    text: "速率"
                    color: autoNegCombo.currentIndex === 1
                           ? root.clSecondary : root.clDisabled
                }

                ComboBox {
                    id: speedCombo
                    Layout.fillWidth: true
                    enabled: autoNegCombo.currentIndex === 1
                    model: [
                        { text: "（未设置）", value: 0 },
                        { text: "10 Mb/s", value: 10 },
                        { text: "100 Mb/s", value: 100 },
                        { text: "1 Gb/s", value: 1000 },
                        { text: "2.5 Gb/s", value: 2500 },
                        { text: "5 Gb/s", value: 5000 },
                        { text: "10 Gb/s", value: 10000 }
                    ]
                    textRole: "text"
                    valueRole: "value"

                    currentIndex: {
                        const v = root.editor.wiredSpeed
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].value === v)
                                return i
                        }
                        return 0
                    }

                    onActivated:
                        root.editor.wiredSpeed =
                            model[currentIndex].value
                }

                // ----- 双工模式（仅手动模式可编辑）-----
                Label {
                    text: "双工模式"
                    color: autoNegCombo.currentIndex === 1
                           ? root.clSecondary : root.clDisabled
                }

                ComboBox {
                    id: duplexCombo
                    Layout.fillWidth: true
                    enabled: autoNegCombo.currentIndex === 1
                    model: [
                        { text: "（未设置）", value: "" },
                        { text: "全双工", value: "full" },
                        { text: "半双工", value: "half" }
                    ]
                    textRole: "text"
                    valueRole: "value"

                    currentIndex: {
                        const v = root.editor.wiredDuplex
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].value === v)
                                return i
                        }
                        return 0
                    }

                    onActivated:
                        root.editor.wiredDuplex =
                            model[currentIndex].value
                }
            }

            // =====================================
            // IPv4
            // =====================================

            Label {
                text: "IPv4 设置"
                font.bold: true
                font.pixelSize: 14
                color: root.clPrimary
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                Label {
                    text: "方法"
                    color: root.clSecondary
                }

                ComboBox {
                    Layout.fillWidth: true
                    model: [
                        { text: "自动 (DHCP)", value: "auto" },
                        { text: "手动", value: "manual" },
                        { text: "禁用", value: "disabled" },
                        { text: "仅链路本地", value: "link-local" },
                        { text: "共享", value: "shared" }
                    ]
                    textRole: "text"
                    valueRole: "value"

                    currentIndex: {
                        const m = root.editor.ipv4Method || "auto"
                        for (let i = 0; i < model.length; ++i) {
                            if (model[i].value === m)
                                return i
                        }
                        return 0
                    }

                    onActivated:
                        root.editor.ipv4Method =
                            model[currentIndex].value
                }

                Label {
                    text: hint("地址", root.editor.ipv4Address)
                    visible: root.editor.ipv4Method === "manual"
                    color: root.editor.ipv4Address ? root.clSecondary : root.clDisabled
                }

                TextField {
                    visible: root.editor.ipv4Method === "manual"
                    Layout.fillWidth: true
                    text: root.editor.ipv4Address
                    placeholderText: root.editor.ipv4Address ? "" : "未配置"
                    placeholderTextColor: root.clDisabled
                    onEditingFinished:
                        root.editor.ipv4Address = text
                }

                Label {
                    text: hint("网关", root.editor.ipv4Gateway)
                    visible: root.editor.ipv4Method === "manual"
                    color: root.editor.ipv4Gateway ? root.clSecondary : root.clDisabled
                }

                TextField {
                    visible: root.editor.ipv4Method === "manual"
                    Layout.fillWidth: true
                    text: root.editor.ipv4Gateway
                    placeholderText: root.editor.ipv4Gateway ? "" : "未配置"
                    placeholderTextColor: root.clDisabled
                    onEditingFinished:
                        root.editor.ipv4Gateway = text
                }

                Label {
                    text: hint("DNS", root.editor.ipv4Dns && root.editor.ipv4Dns.length > 0)
                    color: (root.editor.ipv4Dns && root.editor.ipv4Dns.length > 0) ? root.clSecondary : root.clDisabled
                }

                TextField {
                    Layout.fillWidth: true
                    text: (root.editor.ipv4Dns || []).join(", ")
                    placeholderText: "8.8.8.8, 8.8.4.4"
                    placeholderTextColor: root.clSecondary
                    onEditingFinished:
                        root.editor.ipv4Dns = text.split(",")
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    // =====================================
    // 操作按钮（固定在底部，不跟随滚动）
    // =====================================

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        spacing: 10

        Button {
            text: "取消"
            onClicked: {
                root.editor.reset()
                root.cancelled()
            }
        }

        Button {
            text: "保存"
            enabled: root.editor.isModified
            onClicked: {
                root.manager.apply(
                    root.editor.toSettingInfo(),
                    root.editor.isNew,
                    nm.currentUuid
                )
                root.editor.commit()
                root.saved()
            }
        }
    }
    }
}
