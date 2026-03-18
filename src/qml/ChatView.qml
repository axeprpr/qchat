import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ListView {
    id: chatView
    model: chatManager.messageModel
    spacing: 4
    clip: true
    verticalLayoutDirection: ListView.TopToBottom
    boundsBehavior: Flickable.StopAtBounds

    // Auto-scroll to bottom
    onCountChanged: {
        Qt.callLater(function() { positionViewAtEnd() })
    }

    // Also scroll when streaming
    Connections {
        target: chatManager.messageModel
        function onStreamingUpdated() {
            Qt.callLater(function() { chatView.positionViewAtEnd() })
        }
    }

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        contentItem: Rectangle {
            implicitWidth: 6
            radius: 3
            color: theme.scrollbar
            opacity: parent.active ? 0.8 : 0.4
        }
    }

    header: Item { height: 16 }
    footer: Item { height: 16 }

    delegate: MessageBubble {
        width: chatView.width
        messageRole: model.role
        messageContent: model.content
        thinkingContent: model.thinkingContent || ""
        messageTimestamp: model.timestamp
        isStreaming: model.isStreaming
        attachments: model.attachments || []
        isError: model.isError
    }

    // Empty state handled by parent
}
