import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluContentDialog {
    id: root
    property int conversationIndex: -1
    property var convSettings: ({})

    title: "Conversation Settings"
    positiveText: "Save"
    negativeText: "Cancel"

    onOpened: {
        if (conversationIndex >= 0) {
            convSettings = chatManager.conversationModel.getConversationSettings(conversationIndex)
            nameField.text = convSettings.title || ""
            systemPromptField.text = convSettings.systemPrompt || ""
            tempSlider.value = convSettings.temperature >= 0 ? convSettings.temperature : chatManager.settings.temperature
            paramsField.text = convSettings.parameters || ""
            markdownSwitch.checked = convSettings.markdownEnabled !== false
            historySwitch.checked = convSettings.historyToolEnabled !== false

            var providers = ["OpenAI", "Claude", "Gemini", "DeepSeek", "Ollama"]
            var idx = providers.indexOf(convSettings.provider || "")
            providerCombo.currentIndex = idx >= 0 ? idx : 0
        }
    }

    onPositiveClicked: {
        var providers = ["OpenAI", "Claude", "Gemini", "DeepSeek", "Ollama"]
        chatManager.conversationModel.updateConversationSettings(conversationIndex, {
            "title": nameField.text,
            "provider": providers[providerCombo.currentIndex],
            "systemPrompt": systemPromptField.text,
            "temperature": tempSlider.value,
            "parameters": paramsField.text,
            "markdownEnabled": markdownSwitch.checked,
            "historyToolEnabled": historySwitch.checked
        })
        FluToast.success("Conversation settings saved")
    }

    contentDelegate: Component {
        ColumnLayout {
            width: 480
            spacing: 16

            // Name
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                FluText { text: "Name"; fontSize: FluTextStyle.BodyStrong }
                FluTextBox {
                    id: nameField
                    Layout.fillWidth: true
                    placeholderText: "Conversation name"
                }
            }

            // LLM Provider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                FluText { text: "LLM Provider"; fontSize: FluTextStyle.BodyStrong }
                FluComboBox {
                    id: providerCombo
                    Layout.fillWidth: true
                    model: ["OpenAI", "Claude", "Gemini", "DeepSeek", "Ollama"]
                }
            }

            // System Prompt
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                FluText { text: "System Prompt"; fontSize: FluTextStyle.BodyStrong }
                FluMultilineTextBox {
                    id: systemPromptField
                    Layout.fillWidth: true
                    implicitHeight: 100
                    placeholderText: "Override global system prompt for this conversation..."
                }
            }

            // Temperature
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                RowLayout {
                    Layout.fillWidth: true
                    FluText { text: "Temperature"; fontSize: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                    FluText {
                        text: tempSlider.value.toFixed(2)
                        color: FluTheme.dark ? "#aaa" : "#666"
                        fontSize: FluTextStyle.Caption
                    }
                }
                FluSlider {
                    id: tempSlider
                    Layout.fillWidth: true
                    from: 0; to: 2; stepSize: 0.05
                }
            }

            // Extra Parameters
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                FluText { text: "Extra Parameters (JSON)"; fontSize: FluTextStyle.BodyStrong }
                FluMultilineTextBox {
                    id: paramsField
                    Layout.fillWidth: true
                    implicitHeight: 60
                    placeholderText: '{"top_p": 0.9}'
                }
            }

            // Toggles
            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                RowLayout {
                    FluText { text: "Markdown"; fontSize: FluTextStyle.Body }
                    FluToggleSwitch { id: markdownSwitch; checked: true }
                }

                RowLayout {
                    FluText { text: "History Tool"; fontSize: FluTextStyle.Body }
                    FluToggleSwitch { id: historySwitch; checked: true }
                }
            }
        }
    }
}
