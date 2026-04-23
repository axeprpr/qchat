import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform
import FluentUI

FluPage {
    id: chatPage
    title: chatManager.conversationModel.currentIndex >= 0
        ? (chatManager.conversationModel.data(
               chatManager.conversationModel.index(chatManager.conversationModel.currentIndex, 0),
               Qt.UserRole + 2) || "New Chat")
        : "New Chat"
    property string currentAgentName: ""
    property var currentSkills: []
    property var currentMcpServers: []

    function refreshAgentName() {
        currentAgentName = chatManager.conversationAgentName()
        currentSkills = chatManager.conversationSkillNames()
        currentMcpServers = chatManager.conversationMcpServerNames()
    }

    padding: 0

    // Header
    header: Rectangle {
        width: parent.width
        height: 48
        color: FluTheme.dark ? "#1f1f1f" : "#f9f9f9"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 8
            spacing: 8

            FluText {
                text: chatPage.title
                font: FluTextStyle.Subtitle
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Rectangle {
                visible: currentAgentName.length > 0
                radius: 10
                color: FluTheme.dark ? "#1f3a5f" : "#dbeafe"
                border.width: 1
                border.color: FluTheme.dark ? "#2b5b8a" : "#93c5fd"
                implicitHeight: 28
                implicitWidth: badgeText.implicitWidth + 18

                FluText {
                    id: badgeText
                    anchors.centerIn: parent
                    text: "Agent: " + currentAgentName
                    color: FluTheme.dark ? "#bfdbfe" : "#1d4ed8"
                    font: FluTextStyle.Caption
                }
            }

            Rectangle {
                visible: currentSkills.length > 0
                radius: 10
                color: FluTheme.dark ? "#203a2f" : "#dcfce7"
                border.width: 1
                border.color: FluTheme.dark ? "#2f6a54" : "#86efac"
                implicitHeight: 28
                implicitWidth: skillBadgeText.implicitWidth + 18

                FluText {
                    id: skillBadgeText
                    anchors.centerIn: parent
                    text: "Skills: " + currentSkills.length
                    color: FluTheme.dark ? "#bbf7d0" : "#166534"
                    font: FluTextStyle.Caption
                }
            }

            Rectangle {
                visible: currentMcpServers.length > 0
                radius: 10
                color: FluTheme.dark ? "#3d2f1f" : "#fef3c7"
                border.width: 1
                border.color: FluTheme.dark ? "#7c5a2f" : "#fcd34d"
                implicitHeight: 28
                implicitWidth: mcpBadgeText.implicitWidth + 18

                FluText {
                    id: mcpBadgeText
                    anchors.centerIn: parent
                    text: "MCP: " + currentMcpServers.length
                    color: FluTheme.dark ? "#fde68a" : "#92400e"
                    font: FluTextStyle.Caption
                }
            }

            FluIconButton {
                iconSource: FluentIcons.Settings
                iconSize: 16
                ToolTip.text: "Conversation Settings"
                ToolTip.visible: hovered
                onClicked: convSettingsDialog.open()
            }

            FluIconButton {
                iconSource: FluentIcons.Share
                iconSize: 16
                ToolTip.text: "Export"
                ToolTip.visible: hovered
                onClicked: exportFormatDialog.open()
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: FluTheme.dark ? "#333" : "#e0e0e0"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Message list
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: chatManager.messageModel
            clip: true
            spacing: 4
            topMargin: 12
            bottomMargin: 12

            ScrollBar.vertical: FluScrollBar {}

            delegate: MessageBubble {
                width: messageList.width
                messageRole: model.role
                messageContent: model.content
                thinkingContent: model.thinkingContent
                messageTimestamp: model.timestamp
                isStreaming: model.isStreaming
                attachments: model.attachments
                isError: model.isError
            }

            onCountChanged: {
                Qt.callLater(() => messageList.positionViewAtEnd())
            }
        }

        // Input bar
        ChatInputBar {
            Layout.fillWidth: true
        }
    }

    // Conversation settings dialog
    ConversationSettings {
        id: convSettingsDialog
        conversationIndex: chatManager.conversationModel.currentIndex
    }

    // Export format dialog
    FluContentDialog {
        id: exportFormatDialog
        title: "Export Conversation"
        message: "Choose export format:"
        buttonFlags: FluContentDialogType.NegativeButton | FluContentDialogType.NeutralButton | FluContentDialogType.PositiveButton
        negativeText: "Markdown"
        neutralText: "HTML"
        positiveText: "PDF"
        onNegativeClicked: { exportSaveDialog.exportExt = ".md"; exportSaveDialog.open() }
        onNeutralClicked: { exportSaveDialog.exportExt = ".html"; exportSaveDialog.open() }
        onPositiveClicked: { exportSaveDialog.exportExt = ".pdf"; exportSaveDialog.open() }
    }

    Platform.FileDialog {
        id: exportSaveDialog
        property string exportExt: ".md"
        title: "Save Export"
        fileMode: Platform.FileDialog.SaveFile
        nameFilters: exportExt === ".md" ? ["Markdown (*.md)"] :
                     exportExt === ".html" ? ["HTML (*.html)"] : ["PDF (*.pdf)"]
        onAccepted: {
            var path = file.toString().replace("file:///", "/").replace("file://", "")
            if (!path.endsWith(exportExt)) path += exportExt
            chatManager.exportConversation(path)
        }
    }

    Component.onCompleted: refreshAgentName()

    Connections {
        target: chatManager
        function onCurrentConversationIdChanged() {
            refreshAgentName()
        }
        function onConversationAgentChanged() {
            refreshAgentName()
        }
        function onAgentsChanged() {
            refreshAgentName()
        }
        function onSkillsChanged() {
            refreshAgentName()
        }
        function onMcpServersChanged() {
            refreshAgentName()
        }
        function onConversationCapabilitiesChanged() {
            refreshAgentName()
        }
    }
}
