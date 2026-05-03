import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Networkmanager_GUI_Qt 1.0

Dialog {
    id: dialog

    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    readonly property color clPrimary:   "#1a1a1a"
    readonly property color clSecondary: "#666666"
    readonly property color clDisabled:  "#999999"

    property string viewState: "typeSelect"
    property string selectedType: ""

    width: viewState === "typeSelect" ? 420 : 580
    height: viewState === "typeSelect" ? 240 : 540

    title: viewState === "typeSelect" ? "新建连接" : "编辑连接"

    contentItem.clip: true

    ConnectionEditorModel {
        id: dialogEditor
    }

    function launch() {
        viewState = "typeSelect"
        selectedType = ""
        typeCombo.currentIndex = 0
        dialog.open()
    }

    onClosed: {
        if (viewState === "editor")
            dialogEditor.reset()
        viewState = "typeSelect"
    }

    // ===== 类型选择 =====

    Item {
        visible: viewState === "typeSelect"
        anchors.fill: parent

        ColumnLayout {
            anchors.centerIn: parent
            width: parent.width - 40
            spacing: 12

            Label {
                text: "新建连接"
                font.bold: true
                font.pixelSize: 14
                color: clPrimary
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            Label {
                text: "连接类型"
                color: clSecondary
            }

            ComboBox {
                id: typeCombo
                Layout.fillWidth: true
                model: [
                    { text: "新建 802.11 无线连接", value: "802-11-wireless" },
                    { text: "新建 802.3 以太网连接", value: "802-3-ethernet" }
                ]
                textRole: "text"
                valueRole: "value"
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10

                Button {
                    text: "取消"
                    onClicked: dialog.close()
                }

                Button {
                    text: "下一步"
                    onClicked: {
                        selectedType = typeCombo.currentValue
                        dialogEditor.loadDefaults(selectedType)
                        viewState = "editor"
                    }
                }
            }
        }
    }

    // ===== 编辑器 =====

    ConnectionEditorPanel {
        visible: viewState === "editor"
        anchors.fill: parent
        anchors.margins: 8
        editor: dialogEditor
        manager: nm.manager

        onSaved: dialog.close()
        onCancelled: dialog.close()
    }
}
