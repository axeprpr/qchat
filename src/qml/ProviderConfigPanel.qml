import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluExpander {
    id: root
    property string providerName: ""
    headerText: providerName
    width: parent.width

    property var cfg: chatManager.settings.getProviderConfig(providerName)

    ColumnLayout {
        width: parent.width
        spacing: 12
        padding: 16

        // API Key
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            FluText { text: "API Key"; fontSize: FluTextStyle.BodyStrong }
            FluPasswordBox {
                id: apiKeyField
                Layout.fillWidth: true
                text: root.cfg["apiKey"] || ""
                placeholderText: "Enter API key..."
                onEditingFinished: saveConfig()
            }
        }

        // Base URL
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            FluText { text: "Base URL"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: baseUrlField
                Layout.fillWidth: true
                text: root.cfg["baseUrl"] || ""
                placeholderText: "Leave empty for default"
                onEditingFinished: saveConfig()
            }
        }

        // Default Model
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            FluText { text: "Default Model"; fontSize: FluTextStyle.BodyStrong }
            FluTextBox {
                id: modelField
                Layout.fillWidth: true
                text: root.cfg["defaultModel"] || ""
                placeholderText: "e.g. gpt-4o"
                onEditingFinished: saveConfig()
            }
        }

        // Save button
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
            modelField.text
        )
        FluToast.success(providerName + " settings saved")
    }
}
