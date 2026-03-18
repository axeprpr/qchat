import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    anchors.fill: parent

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 24
        width: Math.min(parent.width - 80, 600)

        // Logo / Title
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "QChat"
            color: theme.primary
            font.pixelSize: 48
            font.bold: true
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Your Private AI Assistant"
            color: theme.textSecondary
            font.pixelSize: 16
        }

        // Feature cards
        GridLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20
            columns: 2
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { icon: "🚀", title: "Multi-Provider", desc: "OpenAI, Claude, DeepSeek, Gemini, Ollama" },
                    { icon: "🧠", title: "Deep Thinking", desc: "Extended reasoning with o1/o3, DeepSeek-R1, Claude" },
                    { icon: "📄", title: "Document Analysis", desc: "Attach and analyze code, text, CSV, JSON files" },
                    { icon: "🔒", title: "Private & Secure", desc: "Your API keys, your data, fully local storage" }
                ]

                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 90
                    radius: theme.radius
                    color: theme.surfaceVariant
                    border.color: cardMouse.containsMouse ? theme.primary : theme.border
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 12

                        Text {
                            text: modelData.icon
                            font.pixelSize: 28
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: modelData.title
                                color: theme.text
                                font.pixelSize: 14
                                font.bold: true
                            }
                            Text {
                                Layout.fillWidth: true
                                text: modelData.desc
                                color: theme.textSecondary
                                font.pixelSize: 12
                                wrapMode: Text.Wrap
                            }
                        }
                    }

                    MouseArea {
                        id: cardMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
        }

        // Quick prompts
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 16
            text: "Try asking..."
            color: theme.textSecondary
            font.pixelSize: 13
        }

        Flow {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            Repeater {
                model: [
                    "Explain quantum computing simply",
                    "Write a Python web scraper",
                    "Help me debug my code",
                    "Translate this to Chinese"
                ]

                delegate: Rectangle {
                    width: promptText.width + 24
                    height: 36
                    radius: 18
                    color: promptMouse.containsMouse ? theme.surfaceVariant : "transparent"
                    border.color: theme.border
                    border.width: 1

                    Text {
                        id: promptText
                        anchors.centerIn: parent
                        text: modelData
                        color: theme.text
                        font.pixelSize: 12
                    }

                    MouseArea {
                        id: promptMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            chatManager.sendMessage(modelData)
                        }
                    }
                }
            }
        }
    }
}
