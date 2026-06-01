import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Item {
    id: root

    // ===== 三级灰度色阶（与 ConnectionEditorPanel 一致）=====
    readonly property color clPrimary:   "#1a1a1a"
    readonly property color clSecondary: "#666666"
    readonly property color clDisabled:  "#999999"

    property string pendingUuid: ""

    // 当前 runtime 对应行
    property int currentRuntimeRow: -1

    // =====================================
    // runtime 行定位
    // =====================================

    function updateCurrentRuntimeRow() {

        currentRuntimeRow = -1

        if (!nm.currentUuid)
            return

        for (let i = 0; i < nm.runtimeModel.rowCount(); ++i) {

            const item = nm.runtimeModel.get(i)

            if (item.uuid === nm.currentUuid) {

                currentRuntimeRow = i

                return
            }
        }
    }

    // =====================================
    // 当前连接变化
    // =====================================

    Connections {
        target: nm

        function onCurrentUuidChanged() {
            updateCurrentRuntimeRow()
        }
    }

    // =====================================
    // runtime model 变化
    // =====================================

    Connections {
        target: nm.runtimeModel

        function onRowsInserted() {
            updateCurrentRuntimeRow()
        }

        function onRowsRemoved() {
            updateCurrentRuntimeRow()
        }

        function onModelReset() {
            updateCurrentRuntimeRow()
        }

        function onStateChanged(uuid) {

            if (uuid === nm.currentUuid) {

                // 强制 delegate 重建
                runtimeRepeater.model = 0
                runtimeRepeater.model = 1
            }
        }
    }

    // =====================================
    // 选择连接
    // =====================================

    function applySelection(uuid) {

        if (!uuid || uuid === nm.currentUuid)
            return

        nm.selectConnection(uuid)
    }

    // =====================================
    // 默认选择
    // =====================================

    function ensureDefaultSelection() {

        if (!listView.model || listView.count <= 0) {

            nm.currentUuid = ""

            return
        }

        if (nm.editor.isModified)
            return

        if (!nm.currentUuid ||
            !nm.hasConnection(nm.currentUuid)) {

            const firstUuid = nm.firstConnectionUuid()

            if (firstUuid)
                applySelection(firstUuid)
        }
    }

    // =====================================
    // 主布局
    // =====================================

    RowLayout {
        anchors.fill: parent
        spacing: 10

        // =====================================
        // 左侧：连接列表
        // =====================================

        Rectangle {

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.minimumWidth: 220

            color: "#C1CEDA"
            radius: 6

            ColumnLayout {

                anchors.fill: parent
                anchors.margins: 8

                spacing: 6

                // =====================================
                // 新增
                // =====================================

                Button {

                    text: "新增连接"

                    Layout.fillWidth: true
                    Layout.preferredHeight: 36

                    onClicked: {

                        if (checkUnsaved())
                            return

                        newConnDialog.launch()
                    }
                }

                // =====================================
                // 列表
                // =====================================

                ListView {

                    id: listView

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    spacing: 6
                    clip: true

                    model: nm.connectionListModel

                    onCountChanged:
                        ensureDefaultSelection()

                    Component.onCompleted:
                        ensureDefaultSelection()

                    delegate: Rectangle {

                        width: listView.width
                        height: 60

                        radius: 6

                        property bool selected:
                            model.conUuid === nm.currentUuid

                        property bool hovered: false

                        color: {
                            if (selected) return "#39d7b0"
                            if (hovered) return "#52A08D"
                            return "transparent"
                        }

                        MouseArea {

                            anchors.fill: parent

                            hoverEnabled: true

                            onEntered: parent.hovered = true
                            onExited:  parent.hovered = false

                            onClicked: {

                                if (model.conUuid
                                        === nm.currentUuid)
                                    return

                                if (checkUnsaved(
                                            model.conUuid))
                                    return

                                applySelection(
                                            model.conUuid)
                            }
                        }

                        RowLayout {

                            anchors.fill: parent
                            anchors.margins: 8

                            Column {

                                spacing: 2

                                Label {

                                    text:
                                        model.conName || ""

                                    font.bold: true

                                    color:
                                        selected
                                        ? "#ffffff"
                                        : ({
                                            2: root.clPrimary,
                                            1: root.clSecondary,
                                            3: root.clSecondary
                                        }[model.conActiveState] || root.clDisabled)
                                }

                                RowLayout {
                                    spacing: 8

                                    Label {
                                        text: {
                                            switch (model.conActiveState) {
                                                case 1: return "正在激活"
                                                case 2: return "已激活"
                                                case 3: return "正在断开"
                                                default: return "未连接"
                                            }
                                        }

                                        color: {
                                            switch (model.conActiveState) {
                                                case 1: return "orange"
                                                case 2: return "green"
                                                case 3: return "orange"
                                                default: return root.clSecondary
                                            }
                                        }

                                        font.pixelSize: 12
                                    }

                                    Label {
                                        text: {
                                            const t = model.conLastConnection
                                            if (!t || t.getTime() <= 0) return ""
                                            const sec = (Date.now() - t.getTime()) / 1000
                                            if (sec < 60)   return "刚刚"
                                            if (sec < 3600)  return Math.floor(sec / 60) + " 分钟前"
                                            if (sec < 86400) return Math.floor(sec / 3600) + " 小时前"
                                            const days = Math.floor(sec / 86400)
                                            if (days < 30)   return days + " 天前"
                                            if (days < 365)  return Math.floor(days / 30) + " 个月前"
                                            return "一年前"
                                        }
                                        color: selected ? "#e0e0e0" : root.clDisabled
                                        font.pixelSize: 11
                                        visible: model.conActiveState !== 2
                                                 && !!model.conLastConnection
                                                 && model.conLastConnection.getTime() > 0
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Button {

                                text: {
                                    switch (model.conActiveState) {
                                        case 1: return "正在激活..."
                                        case 2: return "断开"
                                        case 3: return "正在断开..."
                                        default: return "连接"
                                    }
                                }

                                enabled:
                                    model.conActiveState !== 1
                                    && model.conActiveState !== 3

                                onClicked: {
                                    if (model.conActive)
                                        nm.connectionManager.deactivateConnection(model.conUuid)
                                    else
                                        nm.connectionManager.activateConnection(model.conUuid)
                                }
                            }
                        }
                    }

                    // =====================================
                    // 空列表
                    // =====================================

                    footer: Item {

                        width: parent.width

                        height:
                            listView.count === 0
                            ? 120
                            : 0

                        Label {

                            anchors.centerIn: parent

                            visible:
                                listView.count === 0

                            text: "没有连接配置"

                            color: root.clDisabled
                        }
                    }
                }

                // =====================================
                // 删除
                // =====================================

                Button {

                    text: "删除连接"

                    Layout.fillWidth: true
                    Layout.preferredHeight: 36

                    enabled:
                        nm.currentUuid !== ""

                    onClicked: {

                        nm.manager.deleteConnection(
                                    nm.currentUuid)

                        nm.currentUuid = ""
                    }
                }
            }
        }

        // =====================================
        // 右侧：状态 + 配置
        // =====================================

        Rectangle {

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 3
            Layout.minimumWidth: 400

            color: "#C1CEDA"
            // border.color: "#ddd"

            radius: 6

            ColumnLayout {

                anchors.fill: parent
                spacing: 0

                // =====================================
                // Tab
                // =====================================

                TabBar {
                    id: tabBar

                    Layout.fillWidth: true

                    TabButton {
                        text: "状态"
                    }

                    TabButton {
                        text: "配置"
                    }
                }

                StackLayout {

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex:
                        tabBar.currentIndex

                    // =====================================
                    // 状态页
                    // =====================================

                    Item {

                        ColumnLayout {

                            anchors.fill: parent

                            anchors.margins: 16
                            spacing: 10

                            // ===== 未选择连接 =====

                            Label {

                                visible:
                                    !nm.currentUuid

                                text: "请选择一个连接"

                                color: root.clDisabled

                                font.pixelSize: 14
                            }

                            // ===== 运行时信息 =====

                            Repeater {

                                id: runtimeRepeater

                                model:
                                    currentRuntimeRow >= 0
                                    ? 1
                                    : 0

                                delegate: ColumnLayout {

                                    property var rt:
                                        nm.runtimeModel.get(
                                            currentRuntimeRow)

                                    spacing: 10

                                    Label {

                                        text: "状态: " + (rt.statusText || "—")

                                        color: getStatusColor(rt)

                                        font.bold: isStatusBold(rt)
                                    }

                                    // ===== 通用设备信息 =====

                                    Rectangle {
                                        height: 1
                                        Layout.fillWidth: true
                                        color: "#e0e0e0"
                                        visible: rt.activeState === 2 || rt.activeState === 3
                                    }

                                    Label {
                                        text: "网络接口: " + (rt.interface || "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && !!rt.interface
                                    }

                                    Label {
                                        text: "MAC 地址: " + (rt.mac || "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && !!rt.mac
                                    }

                                    Label {
                                        text: "MTU: " + (rt.mtu > 0 ? rt.mtu : "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.mtu > 0
                                    }

                                    // ===== IP信息 =====

                                    Rectangle {
                                        height: 1
                                        Layout.fillWidth: true
                                        color: "#e0e0e0"
                                        visible: rt.activeState === 2 || rt.activeState === 3
                                    }

                                    Label {

                                        text:
                                            "IPv4: " +
                                            (rt.ipv4 || "—")

                                        color: root.clSecondary
                                        visible: rt.activeState === 2 || rt.activeState === 3
                                    }

                                    Label {

                                        text:
                                            "网关: " +
                                            (rt.gateway || "—")

                                        color: root.clSecondary
                                        visible: rt.activeState === 2 || rt.activeState === 3
                                    }

                                    Label {

                                        text:
                                            "DNS: " +
                                            ((rt.dns || [])
                                             .join(", ") || "—")

                                        color: root.clSecondary
                                        visible: rt.activeState === 2 || rt.activeState === 3
                                    }

                                    // ===== 有线特有 =====

                                    Rectangle {
                                        height: 1
                                        Layout.fillWidth: true
                                        color: "#e0e0e0"
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wired"
                                    }

                                    Label {
                                        text: {
                                            if (rt.wiredSpeed > 0) {
                                                const mbps = rt.wiredSpeed / 1000
                                                if (mbps >= 1000)
                                                    return "端口速率: " + (mbps / 1000).toFixed(1) + " Gb/s"
                                                return "端口速率: " + mbps + " Mb/s"
                                            }
                                            return "端口速率: —"
                                        }
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wired"
                                    }

                                    Label {
                                        text: "载波状态: " + (rt.carrier ? "已连接" : "未连接")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wired"
                                    }

                                    // ===== 无线特有 =====

                                    Rectangle {
                                        height: 1
                                        Layout.fillWidth: true
                                        color: "#e0e0e0"
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless"
                                    }

                                    Label {
                                        text: "SSID: " + (rt.ssid || "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless"
                                    }

                                    Label {
                                        text: {
                                            if (rt.wirelessRate > 0) {
                                                const mbps = (rt.wirelessRate / 1000).toFixed(0)
                                                return "连接速率: " + mbps + " Mb/s"
                                            }
                                            return "连接速率: —"
                                        }
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless"
                                    }

                                    Label {
                                        text: "工作模式: " + (rt.mode || "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless" && !!rt.mode
                                    }

                                    Label {
                                        text: "频段: " + (rt.frequencyBand || "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless" && !!rt.frequencyBand
                                    }

                                    Label {
                                        text: "带宽: " + (rt.bandwidth > 0 ? rt.bandwidth + " MHz" : "—")
                                        color: root.clSecondary
                                        visible: (rt.activeState === 2 || rt.activeState === 3) && rt.deviceType === "wireless" && rt.bandwidth > 0
                                    }
                                }
                            }

                            // ===== 已选择但无运行时（未连接）=====

                            Label {

                                visible:
                                    nm.currentUuid
                                    && currentRuntimeRow < 0

                                text: "该连接当前未激活"

                                color: root.clDisabled
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }

                    // =====================================
                    // 配置页
                    // =====================================

                    Item {

                        ConnectionEditorPanel {

                            anchors.fill: parent

                            editor: nm.editor

                            manager: nm.manager
                        }
                    }
                }
            }
        }
    }

    // =====================================
    // 状态文本
    // =====================================

    function getStatusColor(rt) {

        if (rt.stateReason >= 2)
            return "red"

        switch (rt.activeState) {
            case 1: return "orange"
            case 2: return "green"
            case 3: return "orange"
            default: return root.clSecondary
        }
    }

    function isStatusBold(rt) {

        return rt.stateReason >= 2
            || rt.activeState === 1
            || rt.activeState === 2
            || rt.activeState === 3
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

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        title: "未保存更改"

        standardButtons:
            Dialog.Yes | Dialog.No

        onAccepted: {

            nm.manager.apply(
                nm.editor.toSettingInfo(),
                nm.editor.isNew
            )

            nm.editor.commit()

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

    NewConnectionDialog {
        id: newConnDialog
    }
}