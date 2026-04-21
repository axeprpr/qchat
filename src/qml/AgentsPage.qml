import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluPage {
    id: root
    title: "Agents"

    property var agentsData: []
    property string selectedAgentId: ""
    property var skillsData: []
    property var mcpData: []
    property var selectedSkillIds: []
    property var selectedMcpServerIds: []

    function toggleId(list, id, enabled) {
        var next = list ? list.slice(0) : []
        var idx = next.indexOf(id)
        if (enabled && idx < 0)
            next.push(id)
        if (!enabled && idx >= 0)
            next.splice(idx, 1)
        return next
    }

    function currentAgent() {
        if (!selectedAgentId)
            return ({})
        return chatManager.agentById(selectedAgentId)
    }

    function reloadAgents() {
        agentsData = chatManager.agents()
        skillsData = chatManager.skills()
        mcpData = chatManager.mcpServers()
        if (agentsData.length === 0) {
            selectedAgentId = ""
            clearForm()
            return
        }

        var found = false
        for (var i = 0; i < agentsData.length; i++) {
            if (agentsData[i].id === selectedAgentId) {
                found = true
                break
            }
        }
        if (!found) {
            selectedAgentId = agentsData[0].id
        }
        loadForm(currentAgent())
    }

    function clearForm() {
        idField.text = ""
        nameField.text = ""
        providerCombo.currentIndex = 0
        baseUrlField.text = ""
        apiKeyField.text = ""
        modelField.text = ""
        userIdField.text = ""
        assistantIdField.text = ""
        modeField.text = ""
        modelNameField.text = ""
        agentNameField.text = ""
        authTokenField.text = ""
        selectedSkillIds = []
        selectedMcpServerIds = []
        readonlyHint.visible = false
    }

    function loadForm(agent) {
        idField.text = agent.id || ""
        nameField.text = agent.name || ""
        var p = agent.provider || "Dify"
        providerCombo.currentIndex = p === "DeerFlow" ? 1 : 0
        baseUrlField.text = agent.baseUrl || ""
        apiKeyField.text = agent.apiKey || ""
        modelField.text = agent.model || ""
        userIdField.text = agent.userId || ""
        assistantIdField.text = agent.assistantId || ""
        modeField.text = agent.mode || ""
        modelNameField.text = agent.modelName || ""
        agentNameField.text = agent.agentName || ""
        authTokenField.text = agent.authToken || ""
        selectedSkillIds = agent.skillIds || []
        selectedMcpServerIds = agent.mcpServerIds || []
        readonlyHint.visible = agent.readonly === true
    }

    function collectForm() {
        var current = currentAgent()
        return {
            "id": idField.text,
            "name": nameField.text,
            "provider": providerCombo.currentText,
            "baseUrl": baseUrlField.text,
            "apiKey": apiKeyField.text,
            "model": modelField.text,
            "userId": userIdField.text,
            "assistantId": assistantIdField.text,
            "mode": modeField.text,
            "modelName": modelNameField.text,
            "agentName": agentNameField.text,
            "authToken": authTokenField.text,
            "skillIds": selectedSkillIds,
            "mcpServerIds": selectedMcpServerIds,
            "readonly": current.readonly === true,
            "builtin": current.builtin === true,
            "description": current.description || ""
        }
    }

    Component.onCompleted: reloadAgents()

    Connections {
        target: chatManager
        function onAgentsChanged() {
            reloadAgents()
        }
        function onSkillsChanged() {
            skillsData = chatManager.skills()
        }
        function onMcpServersChanged() {
            mcpData = chatManager.mcpServers()
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
                        text: "Agent List"
                        fontSize: FluTextStyle.Subtitle
                        Layout.fillWidth: true
                    }
                    FluIconButton {
                        iconSource: FluentIcons.Add
                        ToolTip.text: "New Agent"
                        ToolTip.visible: hovered
                        onClicked: {
                            selectedAgentId = ""
                            clearForm()
                            nameField.text = "New Agent"
                        }
                    }
                }

                ListView {
                    id: agentListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: agentsData
                    spacing: 4

                    delegate: Rectangle {
                        width: agentListView.width
                        height: 44
                        radius: 8
                        color: modelData.id === selectedAgentId
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
                                text: modelData.provider || ""
                                color: FluTheme.dark ? "#9ca3af" : "#6b7280"
                                fontSize: FluTextStyle.Caption
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                selectedAgentId = modelData.id
                                loadForm(chatManager.agentById(selectedAgentId))
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
                        text: "Agent Config"
                        fontSize: FluTextStyle.Subtitle
                    }

                    FluText {
                        id: readonlyHint
                        visible: false
                        text: "Builtin agent: delete is disabled"
                        color: FluTheme.dark ? "#f59e0b" : "#b45309"
                        fontSize: FluTextStyle.Caption
                    }

                    FluTextBox {
                        id: idField
                        Layout.fillWidth: true
                        visible: false
                    }

                    FluTextBox {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: "Agent name"
                    }

                    FluComboBox {
                        id: providerCombo
                        Layout.fillWidth: true
                        model: ["Dify", "DeerFlow"]
                    }

                    FluTextBox {
                        id: baseUrlField
                        Layout.fillWidth: true
                        placeholderText: providerCombo.currentText === "Dify"
                            ? "Dify base url, e.g. https://api.dify.ai/v1"
                            : "DeerFlow base url, e.g. http://127.0.0.1:8000"
                    }

                    FluPasswordBox {
                        id: apiKeyField
                        Layout.fillWidth: true
                        placeholderText: providerCombo.currentText === "Dify"
                            ? "Dify API key"
                            : "Auth token (optional)"
                    }

                    FluTextBox {
                        id: modelField
                        Layout.fillWidth: true
                        placeholderText: "Model (optional)"
                    }

                    FluTextBox {
                        id: userIdField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "Dify"
                        placeholderText: "Dify userId (optional)"
                    }

                    FluTextBox {
                        id: assistantIdField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "DeerFlow"
                        placeholderText: "assistantId (required by DeerFlow)"
                    }

                    FluTextBox {
                        id: modeField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "DeerFlow"
                        placeholderText: "mode (optional, default flash)"
                    }

                    FluTextBox {
                        id: modelNameField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "DeerFlow"
                        placeholderText: "modelName (optional)"
                    }

                    FluTextBox {
                        id: agentNameField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "DeerFlow"
                        placeholderText: "agentName (optional)"
                    }

                    FluPasswordBox {
                        id: authTokenField
                        Layout.fillWidth: true
                        visible: providerCombo.currentText === "DeerFlow"
                        placeholderText: "authToken override (optional)"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        FluText {
                            text: "Default Skills"
                            fontSize: FluTextStyle.BodyStrong
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            radius: 8
                            color: FluTheme.dark ? "#262626" : "#f5f5f5"
                            border.width: 1
                            border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                            implicitHeight: skillColumn.implicitHeight + 12

                            Column {
                                id: skillColumn
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 4
                                Repeater {
                                    model: skillsData
                                    delegate: CheckBox {
                                        text: (modelData && modelData.name) ? modelData.name : (modelData.id || "")
                                        checked: selectedSkillIds.indexOf(modelData.id) >= 0
                                        onToggled: {
                                            selectedSkillIds = toggleId(selectedSkillIds, modelData.id, checked)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        FluText {
                            text: "Default MCP Servers"
                            fontSize: FluTextStyle.BodyStrong
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            radius: 8
                            color: FluTheme.dark ? "#262626" : "#f5f5f5"
                            border.width: 1
                            border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                            implicitHeight: mcpColumn.implicitHeight + 12

                            Column {
                                id: mcpColumn
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 4
                                Repeater {
                                    model: mcpData
                                    delegate: CheckBox {
                                        text: (modelData && modelData.name) ? modelData.name : (modelData.id || "")
                                        checked: selectedMcpServerIds.indexOf(modelData.id) >= 0
                                        onToggled: {
                                            selectedMcpServerIds = toggleId(selectedMcpServerIds, modelData.id, checked)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        FluFilledButton {
                            text: "Save Agent"
                            onClicked: {
                                var payload = collectForm()
                                if (!payload.name || payload.name.trim() === "") {
                                    FluToast.error("Agent name is required")
                                    return
                                }
                                if (payload.provider === "DeerFlow" && (!payload.assistantId || payload.assistantId.trim() === "")) {
                                    FluToast.error("assistantId is required for DeerFlow")
                                    return
                                }
                                chatManager.saveAgent(payload)
                                if (!payload.id || payload.id === "") {
                                    reloadAgents()
                                    if (agentsData.length > 0) {
                                        selectedAgentId = agentsData[agentsData.length - 1].id
                                        loadForm(chatManager.agentById(selectedAgentId))
                                    }
                                }
                                FluToast.success("Agent saved")
                            }
                        }

                        FluButton {
                            text: "Delete"
                            enabled: !currentAgent().readonly
                            onClicked: {
                                if (!selectedAgentId) {
                                    return
                                }
                                chatManager.deleteAgent(selectedAgentId)
                                selectedAgentId = ""
                                reloadAgents()
                                FluToast.success("Agent deleted")
                            }
                        }
                    }
                }
            }
        }
    }
}
