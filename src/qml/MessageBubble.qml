import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property string messageRole
    required property string messageContent
    required property string thinkingContent
    required property date messageTimestamp
    required property bool isStreaming
    required property var attachments
    required property bool isError

    height: bubbleColumn.height + 8
    property bool isUser: messageRole === "user"

    ColumnLayout {
        id: bubbleColumn
        width: parent.width
        spacing: 4

        // Role label
        Text {
            Layout.leftMargin: isUser ? 0 : 60
            Layout.rightMargin: isUser ? 60 : 0
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            text: isUser ? "You" : "Assistant"
            color: theme.textSecondary
            font.pixelSize: 11
            font.bold: true
        }

        // Thinking block (for assistant)
        ThinkingBlock {
            Layout.leftMargin: 60
            Layout.rightMargin: 60
            Layout.fillWidth: true
            visible: !isUser && thinkingContent.length > 0
            content: thinkingContent
            isActive: isStreaming && messageContent.length === 0
        }

        // Attached files
        Flow {
            Layout.leftMargin: isUser ? 60 : 60
            Layout.rightMargin: isUser ? 16 : 60
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            spacing: 4
            visible: attachments && attachments.length > 0

            Repeater {
                model: attachments || []
                delegate: Tag {
                    text: modelData.split("/").pop()
                    color: theme.primary
                }
            }
        }

        // Message bubble
        Rectangle {
            Layout.leftMargin: isUser ? 120 : 60
            Layout.rightMargin: isUser ? 16 : 120
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            Layout.maximumWidth: root.width - 180
            Layout.minimumWidth: 60
            width: Math.min(contentText.implicitWidth + 32, Layout.maximumWidth)
            height: contentText.implicitHeight + 24
            radius: theme.radius
            color: isError ? theme.errorBg :
                   isUser ? theme.userBubble : theme.assistantBubble
            border.color: isError ? theme.danger : "transparent"
            border.width: isError ? 1 : 0

            Text {
                id: contentText
                anchors.fill: parent
                anchors.margins: 12
                text: isUser ? messageContent :
                      (messageContent.length > 0 ? chatManager.markdown.toHtml(messageContent) :
                       (isStreaming ? "<span style='color:#888;'>Thinking...</span>" : ""))
                textFormat: isUser ? Text.PlainText : Text.RichText
                color: isUser ? theme.onPrimary : (isError ? theme.danger : theme.text)
                font.pixelSize: theme.fontSizeBase
                wrapMode: Text.Wrap
                lineHeight: 1.5
                onLinkActivated: (link) => Qt.openUrlExternally(link)

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            // Streaming indicator
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 8
                width: 8; height: 8
                radius: 4
                color: theme.primary
                visible: isStreaming

                SequentialAnimation on opacity {
                    running: isStreaming
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
            }
        }

        // Action buttons (for assistant messages)
        Row {
            Layout.leftMargin: 60
            spacing: 4
            visible: !isUser && !isStreaming && messageContent.length > 0

            IconButton {
                width: 28; height: 28
                icon.source: "qrc:/icons/copy.svg"
                icon.width: 14; icon.height: 14
                ToolTip.text: "Copy"
                ToolTip.visible: hovered
                onClicked: {
                    // Copy to clipboard
                    copyHelper.text = messageContent
                    copyHelper.selectAll()
                    copyHelper.copy()
                }
            }

            IconButton {
                width: 28; height: 28
                icon.source: "qrc:/icons/retry.svg"
                icon.width: 14; icon.height: 14
                ToolTip.text: "Retry"
                ToolTip.visible: hovered
                onClicked: chatManager.retryLastMessage()
            }
        }
    }

    // Hidden text field for clipboard
    TextEdit {
        id: copyHelper
        visible: false
    }
}
