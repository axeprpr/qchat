import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluContentDialog {
    id: root
    title: "New Conversation"
    signal conversationCreated()
    property string selectedProvider: chatManager.settings.currentProvider
    property string selectedAgentId: ""
    property var agentIds: [""]
    property var agentNames: ["No Agent"]
    property bool newAgentSession: true
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
        var list = chatManager.agents()
        agentIds = [""]
        agentNames = ["No Agent"]
        for (var i = 0; i < list.length; i++) {
            var item = list[i]
            if (!item || !item.id)
                continue
            agentIds.push(item.id)
            agentNames.push(item.name || item.id)
        }
        agentCombo.model = agentNames
        var idx = agentIds.indexOf(chatManager.conversationAgentId())
        agentCombo.currentIndex = idx >= 0 ? idx : 0
    }

    function reloadCapabilities() {
        skillsData = chatManager.skills()
        mcpData = chatManager.mcpServers()
    }

    function applyAgentDefaults() {
        selectedSkillIds = []
        selectedMcpServerIds = []
        if (selectedAgentId === "") {
            return
        }
        var agent = chatManager.agentById(selectedAgentId)
        selectedSkillIds = agent.skillIds || []
        selectedMcpServerIds = agent.mcpServerIds || []
    }

    positiveText: "Create"
    negativeText: "Cancel"

    onOpened: {
        if (typeof titleField === "undefined")
            return
        titleField.text = ""
        var providers = chatManager.providerNames()
        if (providers.length === 0) {
            providers = ["OpenAI"]
        }
        providerCombo.model = providers
        var idx = providers.indexOf(chatManager.settings.currentProvider)
        providerCombo.currentIndex = idx >= 0 ? idx : 0
        selectedProvider = providerCombo.currentText
        reloadAgents()
        reloadCapabilities()
        selectedAgentId = agentIds[agentCombo.currentIndex] || ""
        applyAgentDefaults()
        var effectiveProvider = selectedProvider
        if (selectedAgentId !== "") {
            var agent = chatManager.agentById(selectedAgentId)
            if (agent && agent.provider) {
                effectiveProvider = agent.provider
            }
        }
        newAgentSession = chatManager.isExternalProvider(effectiveProvider)
        agentSessionSwitch.checked = true
    }

    onPositiveClicked: {
        var effectiveProvider = selectedProvider
        if (selectedAgentId !== "") {
            var selectedAgent = chatManager.agentById(selectedAgentId)
            if (selectedAgent && selectedAgent.provider) {
                effectiveProvider = selectedAgent.provider
            }
        }
        chatManager.newConversationWithOptions(
            titleField.text.trim(),
            effectiveProvider,
            agentSessionSwitch.checked
        )
        if (selectedAgentId !== "") {
            chatManager.setConversationAgentId(selectedAgentId)
        }
        chatManager.setConversationSkillIds(selectedSkillIds)
        chatManager.setConversationMcpServerIds(selectedMcpServerIds)
        conversationCreated()
    }

    contentDelegate: Component {
        ScrollView {
            clip: true
            implicitWidth: 500
            implicitHeight: 620

            ColumnLayout {
                width: parent.width
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText {
                        text: "Title"
                        font: FluTextStyle.BodyStrong
                    }
                    FluTextBox {
                        id: titleField
                        Layout.fillWidth: true
                        placeholderText: "New Chat"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText {
                        text: "Agent"
                        font: FluTextStyle.BodyStrong
                    }
                    FluComboBox {
                        id: agentCombo
                        Layout.fillWidth: true
                        model: ["No Agent"]
                        onCurrentIndexChanged: {
                            selectedAgentId = agentIds[currentIndex] || ""
                            applyAgentDefaults()
                            var effectiveProvider = selectedProvider
                            if (selectedAgentId !== "") {
                                var agent = chatManager.agentById(selectedAgentId)
                                if (agent && agent.provider) {
                                    effectiveProvider = agent.provider
                                }
                            }
                            newAgentSession = chatManager.isExternalProvider(effectiveProvider)
                            agentSessionSwitch.checked = true
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText {
                        text: "Provider"
                        font: FluTextStyle.BodyStrong
                    }
                    FluComboBox {
                        id: providerCombo
                        Layout.fillWidth: true
                        model: []
                        enabled: selectedAgentId === ""
                        onCurrentTextChanged: {
                            selectedProvider = currentText
                            var effectiveProvider = selectedProvider
                            if (selectedAgentId !== "") {
                                var agent = chatManager.agentById(selectedAgentId)
                                if (agent && agent.provider) {
                                    effectiveProvider = agent.provider
                                }
                            }
                            newAgentSession = chatManager.isExternalProvider(effectiveProvider)
                            agentSessionSwitch.checked = true
                        }
                    }
                    FluText {
                        visible: selectedAgentId !== ""
                        text: "Provider is controlled by selected agent"
                        color: FluTheme.dark ? "#9ca3af" : "#6b7280"
                        font: FluTextStyle.Caption
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: newAgentSession
                    FluText {
                        text: "New Agent Session"
                        Layout.fillWidth: true
                        font: FluTextStyle.Body
                    }
                    FluToggleSwitch {
                        id: agentSessionSwitch
                        checked: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    FluText {
                        text: "Skills"
                        font: FluTextStyle.BodyStrong
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: FluTheme.dark ? "#262626" : "#f5f5f5"
                        border.width: 1
                        border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                        implicitHeight: newSkillColumn.implicitHeight + 12

                        Column {
                            id: newSkillColumn
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
                        text: "MCP Servers"
                        font: FluTextStyle.BodyStrong
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: FluTheme.dark ? "#262626" : "#f5f5f5"
                        border.width: 1
                        border.color: FluTheme.dark ? "#333" : "#e5e7eb"
                        implicitHeight: newMcpColumn.implicitHeight + 12

                        Column {
                            id: newMcpColumn
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
            }
        }
    }
}
