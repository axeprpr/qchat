import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluExpander {
    id: root
    property string providerName: ""
    headerText: providerName
    width: parent.width

    property bool isDify: providerName === "Dify"
    property bool isDeerFlow: providerName === "DeerFlow"
    property bool isExternal: isDify || isDeerFlow
    property var cfg: ({})
    property var extra: ({})

    function loadConfig() {
        if (!providerName)
            return

        cfg = chatManager.settings.getProviderConfig(providerName)
        extra = chatManager.settings.getProviderExtra(providerName)

        apiKeyField.text = cfg["apiKey"] || ""
        baseUrlField.text = cfg["baseUrl"] || ""
        modelField.text = cfg["defaultModel"] || ""

        difyUserIdField.text = extra["userId"] || ""
        deerAssistantIdField.text = extra["assistantId"] || ""
        deerModeField.text = extra["mode"] || ""
        deerModelNameField.text = extra["modelName"] || ""
        deerAgentNameField.text = extra["agentName"] || ""
        deerAuthTokenField.text = extra["authToken"] || ""
    }

    Component.onCompleted: loadConfig()
    onProviderNameChanged: loadConfig()

    ColumnLayout {
        width: parent.width
        spacing: 12
        padding: 16

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            FluText {
                text: isDify ? "API Key" : (isDeerFlow ? "Auth Token (Optional)" : "API Key")
                fontSize: FluTextStyle.BodyStrong
            }
            FluPasswordBox {
                id: apiKeyField
                Layout.fillWidth: true
                placeholderText: isDify
                    ? "Dify API key"
                    : (isDeerFlow ? "Bearer token (optional, if gateway requires auth)" : "Enter API key...")
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            FluText { text: "Base URL"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: baseUrlField
                Layout.fillWidth: true
                placeholderText: isDify
                    ? "e.g. https://api.dify.ai/v1"
                    : (isDeerFlow ? "e.g. http://127.0.0.1:8000" : "Leave empty for default")
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDify
            FluText { text: "User ID (Optional)"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: difyUserIdField
                Layout.fillWidth: true
                placeholderText: "Custom user id for Dify session isolation"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDeerFlow
            FluText { text: "Assistant ID"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: deerAssistantIdField
                Layout.fillWidth: true
                placeholderText: "DeerFlow assistant id (required)"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDeerFlow
            FluText { text: "Mode (Optional)"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: deerModeField
                Layout.fillWidth: true
                placeholderText: "flash"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDeerFlow
            FluText { text: "Model Name (Optional)"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: deerModelNameField
                Layout.fillWidth: true
                placeholderText: "model_name passed to DeerFlow context"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDeerFlow
            FluText { text: "Agent Name (Optional)"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: deerAgentNameField
                Layout.fillWidth: true
                placeholderText: "agent_name passed to DeerFlow context"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: isDeerFlow
            FluText { text: "Auth Token Override (Optional)"; fontSize: FluTextStyle.BodyStrong }
            FluPasswordBox {
                id: deerAuthTokenField
                Layout.fillWidth: true
                placeholderText: "If set, this token is preferred over API Key"
                onEditingFinished: saveConfig()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: !isExternal
            FluText { text: "Default Model"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: modelField
                Layout.fillWidth: true
                placeholderText: "e.g. gpt-4o"
                onEditingFinished: saveConfig()
            }
        }

        FluFilledButton {
            text: "Save"
            onClicked: saveConfig()
        }
    }

    function saveConfig() {
        chatManager.settings.saveProviderConfig(
            providerName,
            apiKeyField.text,
            baseUrlField.text,
            isExternal ? "" : modelField.text
        )

        var extras = {}
        if (isDify) {
            extras["userId"] = difyUserIdField.text
        }
        if (isDeerFlow) {
            extras["assistantId"] = deerAssistantIdField.text
            extras["mode"] = deerModeField.text
            extras["modelName"] = deerModelNameField.text
            extras["agentName"] = deerAgentNameField.text
            extras["authToken"] = deerAuthTokenField.text
        }

        chatManager.settings.saveProviderExtra(providerName, extras)
        loadConfig()
        FluToast.success(providerName + " settings saved")
    }
}
