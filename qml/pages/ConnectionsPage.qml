import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {
    id: root
    property string selectedUuid: ""
    property string pendingUuid: ""
    property var runtimeInfo: ({ active: false, ipv4: "", gateway: "", dns: [] })
    onSelectedUuidChanged: refreshRuntime()

    function applySelection(uuid) {
        if (!uuid || uuid === selectedUuid)
            return

        selectedUuid = uuid
        nm.editor.loadByUuid(uuid)
        refreshRuntime()
    }

    function refreshRuntime() {
        if (!selectedUuid) {
            runtimeInfo = ({ active: false, ipv4: "", gateway: "", dns: [] })
            return
        }

        runtimeInfo = nm.runtimeModel.getRuntimeByUuid(selectedUuid)
    }

    function ensureDefaultSelection() {
        if (!listView.model || listView.count <= 0) {
            selectedUuid = ""
            refreshRuntime()
            return
        }

        if (nm.editor.isModified)
            return

        if (!selectedUuid) {
            const firstItem = listView.model.get(0)
            if (firstItem && firstItem.conUuid)
                applySelection(firstItem.conUuid)
            return
        }

        let exists = false
        for (let i = 0; i < listView.count; ++i) {
            const rowItem = listView.model.get(i)
            if (rowItem && rowItem.conUuid === selectedUuid) {
                exists = true
                break
            }
        }

        if (!exists) {
            const firstItem = listView.model.get(0)
            if (firstItem && firstItem.conUuid)
                applySelection(firstItem.conUuid)
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 10

        // =====================================
        // 左侧：连接列表（1份）
        // =====================================
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 1   // ⭐ 关键：比例

            color: "#fafafa"
            border.color: "#ddd"
            radius: 6

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                // ===== 新增 =====
                Button {
                    text: "新增连接"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36

                    onClicked: {
                        if (checkUnsaved()) return
                        nm.editor.loadDefaults("802-11-wireless")
                        selectedUuid = ""
                    }
                }

                // ===== 列表 =====
                ListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 6
                    clip: true

                    model: nm.connectionListModel

                    onCountChanged: ensureDefaultSelection()

                    Component.onCompleted: ensureDefaultSelection()

                    delegate: Rectangle {
                        width: listView.width
                        height: 60
                        radius: 6

                        property bool selected: model.conUuid === selectedUuid
                        color: selected ? "#d0eaff" : "#f5f5f5"

                        border.color: selected ? "#3399ff" : "#ddd"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (model.conUuid === selectedUuid)
                                    return

                                if (checkUnsaved(model.conUuid))
                                    return

                                applySelection(model.conUuid)
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8

                            Column {
                                spacing: 2

                                Label {
                                    text: model.conName || ""
                                    font.bold: true
                                }

                                Label {
                                    text: model.conActive ? "已连接" : "未连接"
                                    color: model.conActive ? "green" : "#666"
                                    font.pixelSize: 12
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: model.conActive ? "断开" : "连接"

                                onClicked: {
                                    if (model.conActive)
                                        nm.manager.deactivate(model.conUuid)
                                    else
                                        nm.manager.activate(model.conUuid)
                                }
                            }
                        }
                    }

                    // 空列表提示
                    footer: Item {
                        width: parent.width
                        height: listView.count === 0 ? 120 : 0

                        Label {
                            anchors.centerIn: parent
                            visible: listView.count === 0
                            text: "没有连接配置"
                            color: "#888"
                        }
                    }
                }

                // ===== 删除 =====
                Button {
                    text: "删除连接"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    enabled: selectedUuid !== ""

                    onClicked: {
                        nm.manager.remove(selectedUuid)
                        selectedUuid = ""
                    }
                }
            }
        }

        // =====================================
        // 右侧：编辑 + 状态（3份）
        // =====================================
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 3   // ⭐ 关键：比例

            color: "white"
            border.color: "#ddd"
            radius: 6

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // ===== Tab =====
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

                    // =====================
                    // 状态页
                    // =====================
                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Label { text: "名称: " + (nm.editor.id || "") }

                            Label {
                                text: "状态: " +
                                      (runtimeInfo.active ? "已连接" : "未连接")
                                color: runtimeInfo.active ? "green" : "#666"
                            }

                            Label { text: "IPv4: " + (runtimeInfo.ipv4 || "") }
                            Label { text: "网关: " + (runtimeInfo.gateway || "") }
                            Label { text: "DNS: " + ((runtimeInfo.dns || []).join(", ")) }

                            Item { Layout.fillHeight: true }
                        }
                    }

                    // =====================
                    // 配置页
                    // =====================
                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            TextField {
                                Layout.fillWidth: true
                                text: nm.editor.id
                                placeholderText: "连接名称"
                                onTextChanged: nm.editor.id = text
                            }

                            CheckBox {
                                text: "自动连接"
                                checked: nm.editor.autoconnect
                                onToggled: nm.editor.autoconnect = checked
                            }

                            Item { Layout.fillHeight: true }

                            RowLayout {
                                Layout.alignment: Qt.AlignRight
                                spacing: 10

                                Button {
                                    text: "取消"
                                    onClicked: nm.editor.reset()
                                }

                                Button {
                                    text: "保存"
                                    enabled: nm.editor.isModified
                                    onClicked: {
                                        nm.manager.apply(
                                            nm.editor.toSettingsMap(),
                                            nm.editor.isNew
                                        )
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // =====================================
    // 未保存提示
    // =====================================
    function checkUnsaved(nextUuid) {
        if (!nm.editor.isModified)
            return false

        pendingUuid = nextUuid || ""
        confirmDialog.open()
        return true
    }

    Dialog {
        id: confirmDialog
        modal: true
        title: "未保存更改"

        standardButtons: Dialog.Yes | Dialog.No

        onAccepted: {
            nm.manager.apply(nm.editor.toSettingsMap(), nm.editor.isNew)
            if (pendingUuid) {
                const uuid = pendingUuid
                pendingUuid = ""
                applySelection(uuid)
            }
        }

        onRejected: {
            nm.editor.reset()
            if (pendingUuid) {
                const uuid = pendingUuid
                pendingUuid = ""
                applySelection(uuid)
            }
        }
    }
}
