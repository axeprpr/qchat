import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string providerName: ""

    radius: theme.radiusSmall
    color: theme.surfaceVariant
    border.color: expanded ? theme.primary : theme.border
    border.width: 1
    height: expanded ? expandedContent.height + headerRow.height + 24 : headerRow.height + 24

    property bool expanded: false

    Behavior on height {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    // Load saved config
    Component.onCompleted: {
        var cfg = chatManager.settings.getProviderConfig(providerName)
        apiKeyField.text = cfg.apiKey || ""
        baseUrlField.text = cfg.baseUrl || ""
        defaultModelField.text = cfg.defaultModel || ""
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Header
        RowLayout {
            id: headerRow
            Layout.fillWidth: true

            Text {
                text: {
                    switch(providerName) {
                        case "OpenAI": return "🟢"
                        case "Claude": return "🟠"
                        case "DeepSeek": return "🔵"
                        case "Gemini": return "🟣"
                        case "Ollama": return "🦙"
                        default: return "⚡"
                    }
                }
                font.pixelSize: 20
            }

            Text {
                text: providerName
                color: theme.text
                font.pixelSize: 15
                font.bold: true
            }

            Text {
                text: providerName === "Ollama" ? "(Local)" : "(Cloud)"
                color: theme.textSecondary
                font.pixelSize: 12
            }

            Item { Layout.fillWidth: true }

            // Status indicator
            Rectangle {
                width: 8; height: 8
                radius: 4
                color: {
                    var cfg = chatManager.settings.getProviderConfig(providerName)
                    return (cfg.apiKey || providerName === "Ollama") ? theme.success : theme.textSecondary
                }
            }

            Text {
                text: expanded ? "▾" : "▸"
                color: theme.textSecondary
                font.pixelSize: 14
            }
        }

        // Expandable content
        ColumnLayout {
            id: expandedContent
            Layout.fillWidth: true
            spacing: 10
            visible: expanded
            opacity: expanded ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            // API Key
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: providerName !== "Ollama"

                Text {
                    text: "API Key"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                TextField {
                    id: apiKeyField
                    Layout.fillWidth: true
                    echoMode: TextInput.Password
                    placeholderText: "sk-..."
                    placeholderTextColor: theme.textSecondary
                    color: theme.text
                    font.pixelSize: 13
                    onTextChanged: saveConfig()
                    background: Rectangle {
                        radius: theme.radiusSmall
                        color: theme.inputBg
                        border.color: theme.border
                    }
                    padding: 8
                }
            }

            // Base URL
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: providerName === "Ollama" ? "Ollama Server URL" : "Base URL (optional)"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                TextField {
                    id: baseUrlField
                    Layout.fillWidth: true
                    placeholderText: {
                        switch(providerName) {
                            case "OpenAI": return "https://api.openai.com/v1"
                            case "Claude": return "https://api.anthropic.com/v1"
                            case "DeepSeek": return "https://api.deepseek.com/v1"
                            case "Gemini": return "https://generativelanguage.googleapis.com/v1beta/openai"
                            case "Ollama": return "http://localhost:11434"
                            default: return ""
                        }
                    }
                    placeholderTextColor: theme.textSecondary
                    color: theme.text
                    font.pixelSize: 13
                    onTextChanged: saveConfig()
                    background: Rectangle {
                        radius: theme.radiusSmall
                        color: theme.inputBg
                        border.color: theme.border
                    }
                    padding: 8
                }
            }

            // Default Model
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: "Default Model"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                TextField {
                    id: defaultModelField
                    Layout.fillWidth: true
                    placeholderText: {
                        switch(providerName) {
                            case "OpenAI": return "gpt-4o"
                            case "Claude": return "claude-sonnet-4-6"
                            case "DeepSeek": return "deepseek-chat"
                            case "Gemini": return "gemini-2.5-pro"
                            case "Ollama": return "llama3.3"
                            default: return ""
                        }
                    }
                    placeholderTextColor: theme.textSecondary
                    color: theme.text
                    font.pixelSize: 13
                    onTextChanged: saveConfig()
                    background: Rectangle {
                        radius: theme.radiusSmall
                        color: theme.inputBg
                        border.color: theme.border
                    }
                    padding: 8
                }
            }
        }
    }

    MouseArea {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: headerRow.height + 24
        cursorShape: Qt.PointingHandCursor
        onClicked: expanded = !expanded
    }

    function saveConfig() {
        chatManager.settings.saveProviderConfig(
            providerName, apiKeyField.text, baseUrlField.text, defaultModelField.text)
    }
}
