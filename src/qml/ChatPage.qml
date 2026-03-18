import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluPage {
    id: chatPage
    title: chatManager.conversationModel.currentIndex >= 0
        ? (chatManager.conversationModel.data(
               chatManager.conversationModel.index(chatManager.conversationModel.currentIndex, 0),
               Qt.UserRole + 2) || "New Chat")
        : "New Chat"

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
                fontSize: FluTextStyle.Subtitle
                Layout.fillWidth: true
                elide: Text.ElideRight
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
                onClicked: {
                    var path = chatManager.settings.dataPath + "/export.md"
                    chatManager.exportConversation(path)
                    FluToast.success("Exported to " + path)
                }
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
}
