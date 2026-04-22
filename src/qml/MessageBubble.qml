import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

Item {
    id: root
    required property string messageRole
    required property string messageContent
    required property string thinkingContent
    required property date messageTimestamp
    required property bool isStreaming
    required property var attachments
    required property bool isError

    property bool isUser: messageRole === "user"
    height: bubbleColumn.implicitHeight + 16

    ColumnLayout {
        id: bubbleColumn
        width: parent.width - 32
        x: 16
        spacing: 4

        // Role label
        FluText {
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            text: isUser ? "You" : "Assistant"
            color: FluTheme.dark ? "#aaa" : "#666"
            fontSize: FluTextStyle.Caption
        }

        // Thinking block
        ThinkingBlock {
            Layout.fillWidth: true
            Layout.leftMargin: isUser ? 80 : 0
            Layout.rightMargin: isUser ? 0 : 80
            visible: !isUser && thinkingContent.length > 0
            content: thinkingContent
            isActive: isStreaming && messageContent.length === 0
        }

        // Attached files
        Flow {
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            spacing: 4
            visible: attachments && attachments.length > 0

            Repeater {
                model: attachments || []
                delegate: FluButton {
                    text: modelData.split("/").pop()
                    iconSource: FluentIcons.Attach
                    enabled: false
                }
            }
        }

        // Message bubble
        Rectangle {
            Layout.alignment: isUser ? Qt.AlignRight : Qt.AlignLeft
            Layout.maximumWidth: root.width * 0.72
            Layout.minimumWidth: 60
            width: Math.min(msgText.implicitWidth + 28, Layout.maximumWidth)
            height: msgText.implicitHeight + 24
            radius: 8
            color: isError
                ? (FluTheme.dark ? "#3d1a1a" : "#fff0f0")
                : isUser
                    ? FluTheme.primaryColor
                    : (FluTheme.dark ? "#2d2d2d" : "#f0f0f5")
            border.color: isError ? "#e74c3c" : "transparent"
            border.width: isError ? 1 : 0

            FluText {
                id: msgText
                anchors {
                    fill: parent
                    margins: 12
                }
                text: isUser
                    ? messageContent
                    : (messageContent.length > 0
                        ? chatManager.markdown.toHtml(messageContent)
                        : (isStreaming ? "<i style='color:#888'>Thinking...</i>" : ""))
                textFormat: isUser ? Text.PlainText : Text.RichText
                color: isError
                    ? "#e74c3c"
                    : isUser ? "#ffffff" : (FluTheme.dark ? "#e0e0e0" : "#1a1a1a")
                wrapMode: Text.Wrap
                lineHeight: 1.5
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            // Streaming dot
            Rectangle {
                anchors { bottom: parent.bottom; right: parent.right; margins: 8 }
                width: 8; height: 8; radius: 4
                color: FluTheme.primaryColor
                visible: isStreaming

                SequentialAnimation on opacity {
                    running: isStreaming
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
            }
        }

        // Action buttons (assistant only)
        Row {
            Layout.alignment: Qt.AlignLeft
            spacing: 4
            visible: !isUser && !isStreaming && messageContent.length > 0

            FluIconButton {
                width: 28; height: 28
                iconSource: FluentIcons.Copy
                iconSize: 14
                ToolTip.text: "Copy"
                ToolTip.visible: hovered
                onClicked: {
                    copyHelper.text = messageContent
                    copyHelper.selectAll()
                    copyHelper.copy()
                    FluToast.success("Copied")
                }
            }

            FluIconButton {
                width: 28; height: 28
                iconSource: FluentIcons.Refresh
                iconSize: 14
                ToolTip.text: "Retry"
                ToolTip.visible: hovered
                onClicked: chatManager.retryLastMessage()
            }
        }
    }

    TextEdit {
        id: copyHelper
        visible: false
    }
}
