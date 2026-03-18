import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluScrollablePage {
    id: callsPage
    title: "Voice Chat"

    property bool isListening: false
    property bool isSpeaking: false
    property string statusText: "Ready"

    ColumnLayout {
        width: Math.min(parent.width - 48, 720)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 24

        Item { height: 8 }

        // Status area
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            FluProgressRing {
                Layout.alignment: Qt.AlignHCenter
                width: 48; height: 48
                visible: isListening || isSpeaking
            }

            FluIcon {
                Layout.alignment: Qt.AlignHCenter
                iconSource: FluentIcons.Microphone
                iconSize: 48
                color: isListening ? FluTheme.primaryColor : (FluTheme.dark ? "#aaa" : "#666")
                visible: !isListening && !isSpeaking
            }

            FluText {
                Layout.alignment: Qt.AlignHCenter
                text: statusText
                fontSize: FluTextStyle.Subtitle
            }
        }

        // Voice messages list
        ListView {
            id: voiceList
            Layout.fillWidth: true
            implicitHeight: Math.min(contentHeight, 400)
            model: ListModel { id: voiceMessages }
            clip: true
            spacing: 4

            ScrollBar.vertical: FluScrollBar {}

            delegate: MessageBubble {
                width: voiceList.width
                messageRole: model.role
                messageContent: model.content
                thinkingContent: ""
                messageTimestamp: new Date()
                isStreaming: false
                attachments: []
                isError: false
            }
        }

        // Controls
        FluFrame {
            Layout.fillWidth: true
            padding: 16

            RowLayout {
                anchors.fill: parent
                spacing: 12

                // Record button
                FluFilledButton {
                    text: isListening ? "Stop Recording" : "Start Recording"
                    iconSource: isListening ? FluentIcons.Stop : FluentIcons.Microphone
                    onClicked: {
                        if (isListening) {
                            isListening = false
                            statusText = "Processing..."
                            // voiceManager.stopRecording()
                        } else {
                            isListening = true
                            statusText = "Listening..."
                            // voiceManager.startRecording()
                        }
                    }
                }

                // Voice selector
                FluComboBox {
                    id: voiceCombo
                    model: ["alloy", "echo", "fable", "onyx", "nova", "shimmer"]
                    implicitWidth: 120
                    ToolTip.text: "TTS Voice"
                    ToolTip.visible: hovered
                }

                Item { Layout.fillWidth: true }

                // Stop speaking
                FluButton {
                    text: "Stop"
                    iconSource: FluentIcons.Stop
                    visible: isSpeaking
                    onClicked: {
                        isSpeaking = false
                        statusText = "Ready"
                        // voiceManager.stopSpeaking()
                    }
                }

                // Clear
                FluIconButton {
                    iconSource: FluentIcons.Delete
                    iconSize: 18
                    ToolTip.text: "Clear history"
                    ToolTip.visible: hovered
                    onClicked: voiceMessages.clear()
                }
            }
        }

        FluText {
            Layout.alignment: Qt.AlignHCenter
            text: "Configure your OpenAI API key in Settings to enable voice features."
            color: FluTheme.dark ? "#888" : "#999"
            fontSize: FluTextStyle.Caption
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Item { height: 8 }
    }
}
