import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluPage {
    id: root
    title: "MCP"

    property var mcpData: []
    property string selectedMcpId: ""

    function currentMcp() {
        if (!selectedMcpId)
            return ({})
        return chatManager.mcpServerById(selectedMcpId)
    }

    function reloadMcp() {
        mcpData = chatManager.mcpServers()
        if (mcpData.length === 0) {
            selectedMcpId = ""
            clearForm()
            return
        }

        var found = false
        for (var i = 0; i < mcpData.length; i++) {
            if (mcpData[i].id === selectedMcpId) {
                found = true
                break
            }
        }
        if (!found) {
            selectedMcpId = mcpData[0].id
        }
        loadForm(currentMcp())
    }

    function clearForm() {
        idField.text = ""
        nameField.text = ""
        transportCombo.currentIndex = 0
        urlField.text = ""
        commandField.text = ""
        argsField.text = ""
        envField.text = "{}"
        descriptionField.text = ""
        enabledSwitch.checked = true
        readonlyHint.visible = false
    }

    function loadForm(mcp) {
        idField.text = mcp.id || ""
        nameField.text = mcp.name || ""
        var transport = mcp.transport || "sse"
        var transportIdx = ["sse", "http", "stdio"].indexOf(transport)
        transportCombo.currentIndex = transportIdx >= 0 ? transportIdx : 0
        urlField.text = mcp.url || ""
        commandField.text = mcp.command || ""
        argsField.text = mcp.args || ""
        envField.text = mcp.envJson || "{}"
        descriptionField.text = mcp.description || ""
        enabledSwitch.checked = mcp.enabled !== false
        readonlyHint.visible = mcp.readonly === true
    }

    function collectForm() {
        var current = currentMcp()
        return {
            "id": idField.text,
            "name": nameField.text,
            "transport": transportCombo.currentText,
            "url": urlField.text,
            "command": commandField.text,
            "args": argsField.text,
            "envJson": envField.text,
            "description": descriptionField.text,
            "enabled": enabledSwitch.checked,
            "readonly": current.readonly === true,
            "builtin": current.builtin === true
        }
    }

    Component.onCompleted: reloadMcp()

    Connections {
        target: chatManager
        function onMcpServersChanged() {
            reloadMcp()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        FluFrame {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            padding: 12

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: "MCP List"
                        font: FluTextStyle.Subtitle
                        Layout.fillWidth: true
                    }
                    FluIconButton {
                        iconSource: FluentIcons.Add
                        ToolTip.text: "New MCP"
                        ToolTip.visible: hovered
                        onClicked: {
                            selectedMcpId = ""
                            clearForm()
                            nameField.text = "New MCP"
                        }
                    }
                }

                ListView {
                    id: mcpListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: mcpData
                    spacing: 4

                    delegate: Rectangle {
                        width: mcpListView.width
                        height: 44
                        radius: 8
                        color: modelData.id === selectedMcpId
                               ? (FluTheme.dark ? "#2a3b57" : "#dbeafe")
                               : (FluTheme.dark ? "#262626" : "#f5f5f5")

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            FluText {
                                text: modelData.name || modelData.id
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            FluText {
                                text: modelData.transport || ""
                                color: FluTheme.dark ? "#9ca3af" : "#6b7280"
                                font: FluTextStyle.Caption
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                selectedMcpId = modelData.id
                                loadForm(chatManager.mcpServerById(selectedMcpId))
                            }
                        }
                    }
                }
            }
        }

        FluFrame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 16

            ScrollView {
                anchors.fill: parent
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    FluText {
                        text: "MCP Config"
                        font: FluTextStyle.Subtitle
                    }

                    FluText {
                        id: readonlyHint
                        visible: false
                        text: "Readonly MCP: delete is disabled"
                        color: FluTheme.dark ? "#f59e0b" : "#b45309"
                        font: FluTextStyle.Caption
                    }

                    FluTextBox {
                        id: idField
                        Layout.fillWidth: true
                        visible: false
                    }

                    FluTextBox {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: "MCP name"
                    }

                    FluComboBox {
                        id: transportCombo
                        Layout.fillWidth: true
                        model: ["sse", "http", "stdio"]
                    }

                    FluTextBox {
                        id: urlField
                        Layout.fillWidth: true
                        placeholderText: "Endpoint URL (for sse/http)"
                    }

                    FluTextBox {
                        id: commandField
                        Layout.fillWidth: true
                        placeholderText: "Command (for stdio)"
                    }

                    FluTextBox {
                        id: argsField
                        Layout.fillWidth: true
                        placeholderText: "Args (space-separated)"
                    }

                    FluMultilineTextBox {
                        id: envField
                        Layout.fillWidth: true
                        implicitHeight: 70
                        placeholderText: "Env JSON, e.g. {\"TOKEN\":\"xxx\"}"
                    }

                    FluMultilineTextBox {
                        id: descriptionField
                        Layout.fillWidth: true
                        implicitHeight: 70
                        placeholderText: "Description"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        FluText {
                            text: "Enabled"
                            Layout.fillWidth: true
                        }
                        FluToggleSwitch {
                            id: enabledSwitch
                            checked: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        FluFilledButton {
                            text: "Save MCP"
                            onClicked: {
                                var payload = collectForm()
                                if (!payload.name || payload.name.trim() === "") {
                                    FluToast.error("MCP name is required")
                                    return
                                }
                                var wasNew = !payload.id || payload.id === ""
                                chatManager.saveMcpServer(payload)
                                reloadMcp()
                                if (wasNew && mcpData.length > 0) {
                                    selectedMcpId = mcpData[mcpData.length - 1].id
                                    loadForm(chatManager.mcpServerById(selectedMcpId))
                                }
                                FluToast.success("MCP saved")
                            }
                        }

                        FluButton {
                            text: "Delete"
                            enabled: !currentMcp().readonly
                            onClicked: {
                                if (!selectedMcpId)
                                    return
                                chatManager.deleteMcpServer(selectedMcpId)
                                selectedMcpId = ""
                                reloadMcp()
                                FluToast.success("MCP deleted")
                            }
                        }
                    }
                }
            }
        }
    }
}
