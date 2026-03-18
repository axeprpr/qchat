import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    color: theme.sidebarBg
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // New chat button
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            Layout.margins: 12
            Layout.bottomMargin: 6
            radius: theme.radius
            color: theme.primary

            RowLayout {
                anchors.centerIn: parent
                spacing: 8

                Image {
                    source: "qrc:/icons/plus.svg"
                    width: 18; height: 18
                    sourceSize: Qt.size(18, 18)
                }

                Text {
                    text: "New Chat"
                    color: theme.primaryText
                    font.pixelSize: 14
                    font.bold: true
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: chatManager.newConversation()

                Rectangle {
                    anchors.fill: parent
                    radius: theme.radius
                    color: "white"
                    opacity: parent.pressed ? 0.15 : parent.containsMouse ? 0.08 : 0
                }
            }
        }

        // Search
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.bottomMargin: 8
            radius: theme.radiusSmall
            color: theme.surfaceVariant
            border.color: searchField.activeFocus ? theme.primary : "transparent"

            TextField {
                id: searchField
                anchors.fill: parent
                placeholderText: "Search conversations..."
                placeholderTextColor: theme.textSecondary
                color: theme.text
                font.pixelSize: 12
                leftPadding: 12
                background: null
            }
        }

        // Conversations list
        ListView {
            id: convList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: chatManager.conversationModel
            clip: true
            currentIndex: chatManager.conversationModel.currentIndex
            spacing: 2

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                contentItem: Rectangle {
                    implicitWidth: 4
                    radius: 2
                    color: theme.scrollbar
                    opacity: parent.active ? 0.8 : 0.3
                }
            }

            delegate: Rectangle {
                id: convDelegate
                width: convList.width - 16
                height: 64
                x: 8
                radius: theme.radiusSmall
                color: convList.currentIndex === index ? theme.surfaceVariant :
                       convMouse.containsMouse ? Qt.rgba(theme.surfaceVariant.r, theme.surfaceVariant.g, theme.surfaceVariant.b, 0.5) : "transparent"

                property bool isHovered: convMouse.containsMouse

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: model.title || "New Chat"
                            color: theme.text
                            font.pixelSize: 13
                            font.bold: convList.currentIndex === index
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }

                        Text {
                            text: model.messageCount > 0 ? model.messageCount : ""
                            color: theme.textSecondary
                            font.pixelSize: 10
                            visible: text.length > 0
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: model.lastMessage || "No messages yet"
                        color: theme.textSecondary
                        font.pixelSize: 11
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }
                }

                // Delete button
                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 6
                    width: 24; height: 24
                    radius: 12
                    color: delMouse.containsMouse ? theme.danger : "transparent"
                    visible: convDelegate.isHovered
                    opacity: 0.8

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: delMouse.containsMouse ? "white" : theme.textSecondary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    MouseArea {
                        id: delMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: chatManager.deleteConversation(index)
                    }
                }

                MouseArea {
                    id: convMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: chatManager.switchConversation(index)
                }
            }
        }

        // Bottom bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: theme.border
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "QChat v1.0"
                    color: theme.textSecondary
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: chatManager.conversationModel.count + " chats"
                    color: theme.textSecondary
                    font.pixelSize: 11
                }
            }
        }
    }
}
