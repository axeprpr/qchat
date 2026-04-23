import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluPage {
    id: welcomePage
    title: "Welcome"
    padding: 0

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 24
        width: Math.min(parent.width - 80, 560)

        // Logo + title
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/icons/logo.svg"
                width: 64; height: 64
                sourceSize: Qt.size(64, 64)
            }

            FluText {
                Layout.alignment: Qt.AlignHCenter
                text: "Welcome to QChat"
                font: FluTextStyle.Title
            }

            FluText {
                Layout.alignment: Qt.AlignHCenter
                text: "A powerful AI chat client with FluentUI design"
                color: FluTheme.dark ? "#aaa" : "#666"
                font: FluTextStyle.Body
            }
        }

        // Feature cards
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { icon: FluentIcons.ChatBubbles,       title: "Multi-Provider",  desc: "OpenAI, Claude, Gemini, DeepSeek & more" },
                    { icon: FluentIcons.Processing,  title: "Deep Thinking",   desc: "Extended reasoning with thinking blocks" },
                    { icon: FluentIcons.Document,    title: "File Attachments",desc: "Attach documents, code, and images" },
                    { icon: FluentIcons.Phone,       title: "Voice Chat",      desc: "STT & TTS powered voice conversations" },
                ]
                delegate: FluFrame {
                    Layout.fillWidth: true
                    padding: 16

                    RowLayout {
                        anchors.fill: parent
                        spacing: 12

                        FluIcon {
                            iconSource: modelData.icon
                            iconSize: 24
                            color: FluTheme.primaryColor
                        }

                        ColumnLayout {
                            spacing: 2
                            FluText {
                                text: modelData.title
                                font: FluTextStyle.BodyStrong
                            }
                            FluText {
                                text: modelData.desc
                                color: FluTheme.dark ? "#aaa" : "#666"
                                font: FluTextStyle.Caption
                                wrapMode: Text.Wrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }

        // New chat button
        FluFilledButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Start New Chat"
            onClicked: chatManager.newConversation()
        }
    }
}
