import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluContentDialog {
    id: root
    property int conversationIndex: -1
    property var convSettings: ({})
    property var providers: chatManager.providerNames()
    property var agentList: []
    property var agentIds: [""]
    property var agentNames: ["No Agent"]
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

    function reloadAgents() {
        agentList = chatManager.agents()
        agentIds = [""]
        agentNames = ["No Agent"]
        for (var i = 0; i < agentList.length; i++) {
            var item = agentList[i]
            if (!item || !item.id)
                continue
            agentIds.push(item.id)
            agentNames.push(item.name || item.id)
        }
    }

    function reloadCapabilities() {
        skillsData = chatManager.skills()
        mcpData = chatManager.mcpServers()
    }

    title: "Conversation Settings"
    positiveText: "Save"
    negativeText: "Cancel"

    onOpened: {
        reloadAgents()
        reloadCapabilities()
        if (conversationIndex >= 0) {
            convSettings = chatManager.conversationModel.getConversationSettings(conversationIndex)
            nameField.text = convSettings.title || ""
            systemPromptField.text = convSettings.systemPrompt || ""
            tempSlider.value = convSettings.temperature >= 0 ? convSettings.temperature : chatManager.settings.temperature
            paramsField.text = convSettings.parameters || ""
            markdownSwitch.checked = convSettings.markdownEnabled !== false
            historySwitch.checked = convSettings.historyToolEnabled !== false

            var idx = providers.indexOf(convSettings.provider || "")
            providerCombo.currentIndex = idx >= 0 ? idx : 0

            var agentIdx = agentIds.indexOf(convSettings.agentId || "")
            agentCombo.currentIndex = agentIdx >= 0 ? agentIdx : 0

            selectedSkillIds = convSettings.skillIds || []
            selectedMcpServerIds = convSettings.mcpServerIds || []
            if (selectedSkillIds.length === 0 && selectedMcpServerIds.length === 0 && convSettings.agentId) {
                var agent = chatManager.agentById(convSettings.agentId)
                selectedSkillIds = agent.skillIds || []
                selectedMcpServerIds = agent.mcpServerIds || []
            }
        }
    }

    onPositiveClicked: {
        var selectedAgentId = agentIds[agentCombo.currentIndex] || ""
        var finalProvider = providers[providerCombo.currentIndex]
        if (selectedAgentId !== "") {
            var agent = chatManager.agentById(selectedAgentId)
            if (agent && agent.provider) {
                finalProvider = agent.provider
            }
        }

        chatManager.conversationModel.updateConversationSettings(conversationIndex, {
            "title": nameField.text,
            "provider": finalProvider,
            "agentId": selectedAgentId,
            "systemPrompt": systemPromptField.text,
            "temperature": tempSlider.value,
            "parameters": paramsField.text,
            "markdownEnabled": markdownSwitch.checked,
            "historyToolEnabled": historySwitch.checked,
            "skillIds": selectedSkillIds,
            "mcpServerIds": selectedMcpServerIds
        })

        if (conversationIndex === chatManager.conversationModel.currentIndex) {
            chatManager.setConversationAgentId(selectedAgentId)
            chatManager.setConversationSkillIds(selectedSkillIds)
            chatManager.setConversationMcpServerIds(selectedMcpServerIds)
        }
        FluToast.success("Conversation settings saved")
    }

    contentDelegate: Component {
        ScrollView {
            clip: true
            implicitWidth: 540
            implicitHeight: 680

            ColumnLayout {
                width: parent.width
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "Name"; font: FluTextStyle.BodyStrong }
                    FluTextBox {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: "Conversation name"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "LLM Provider"; font: FluTextStyle.BodyStrong }
                    FluComboBox {
                        id: providerCombo
                        Layout.fillWidth: true
                        model: providers
                        enabled: agentCombo.currentIndex === 0
                    }
                    FluText {
                        visible: agentCombo.currentIndex > 0
                        text: "Provider is controlled by selected agent"
                        color: FluTheme.dark ? "#9ca3af" : "#6b7280"
                        font: FluTextStyle.Caption
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "Agent"; font: FluTextStyle.BodyStrong }
                    FluComboBox {
                        id: agentCombo
                        Layout.fillWidth: true
                        model: agentNames
                        onCurrentIndexChanged: {
                            var selectedAgentId = agentIds[currentIndex] || ""
                            if (selectedAgentId === "")
                                return
                            var agent = chatManager.agentById(selectedAgentId)
                            selectedSkillIds = agent.skillIds || selectedSkillIds
                            selectedMcpServerIds = agent.mcpServerIds || selectedMcpServerIds
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "Skills"; font: FluTextStyle.BodyStrong }
                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: FluTheme.dark ? "#262626" : "#f5f5f5"
                        border.width: 1
                        border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                        implicitHeight: convSkillColumn.implicitHeight + 12

                        Column {
                            id: convSkillColumn
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
                    FluText { text: "MCP Servers"; font: FluTextStyle.BodyStrong }
                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: FluTheme.dark ? "#262626" : "#f5f5f5"
                        border.width: 1
                        border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                        implicitHeight: convMcpColumn.implicitHeight + 12

                        Column {
                            id: convMcpColumn
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

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "System Prompt"; font: FluTextStyle.BodyStrong }
                    FluMultilineTextBox {
                        id: systemPromptField
                        Layout.fillWidth: true
                        implicitHeight: 100
                        placeholderText: "Override global system prompt for this conversation..."
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        FluText { text: "Temperature"; font: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                        FluText {
                            text: tempSlider.value.toFixed(2)
                            color: FluTheme.dark ? "#aaa" : "#666"
                            font: FluTextStyle.Caption
                        }
                    }
                    FluSlider {
                        id: tempSlider
                        Layout.fillWidth: true
                        from: 0; to: 2; stepSize: 0.05
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText { text: "Extra Parameters (JSON)"; font: FluTextStyle.BodyStrong }
                    FluMultilineTextBox {
                        id: paramsField
                        Layout.fillWidth: true
                        implicitHeight: 60
                        placeholderText: '{"top_p": 0.9}'
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 24

                    RowLayout {
                        FluText { text: "Markdown"; font: FluTextStyle.Body }
                        FluToggleSwitch { id: markdownSwitch; checked: true }
                    }

                    RowLayout {
                        FluText { text: "History Tool"; font: FluTextStyle.Body }
                        FluToggleSwitch { id: historySwitch; checked: true }
                    }
                }
            }
        }
    }
}
