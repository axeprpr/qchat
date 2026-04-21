import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform
import FluentUI

FluScrollablePage {
    title: "Image Generation"

    ColumnLayout {
        width: Math.min(parent.width, 720)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 16

        // Header
        FluText {
            text: "DALL-E 3 Image Generation"
            fontSize: FluTextStyle.Title
        }

        // Prompt input
        FluFrame {
            Layout.fillWidth: true
            padding: 16

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                FluText {
                    text: "Prompt"
                    fontSize: FluTextStyle.BodyStrong
                }

                FluMultilineTextBox {
                    id: promptInput
                    Layout.fillWidth: true
                    implicitHeight: 100
                    placeholderText: "Describe the image you want to generate..."
                    enabled: !chatManager.imageGen.isGenerating
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    ColumnLayout {
                        spacing: 4
                        FluText { text: "Size"; fontSize: FluTextStyle.Caption }
                        FluComboBox {
                            id: sizeCombo
                            model: ["1024x1024", "1792x1024", "1024x1792"]
                            enabled: !chatManager.imageGen.isGenerating
                        }
                    }

                    ColumnLayout {
                        spacing: 4
                        FluText { text: "Quality"; fontSize: FluTextStyle.Caption }
                        FluComboBox {
                            id: qualityCombo
                            model: ["standard", "hd"]
                            enabled: !chatManager.imageGen.isGenerating
                        }
                    }

                    ColumnLayout {
                        spacing: 4
                        FluText { text: "Style"; fontSize: FluTextStyle.Caption }
                        FluComboBox {
                            id: styleCombo
                            model: ["vivid", "natural"]
                            enabled: !chatManager.imageGen.isGenerating
                        }
                    }

                    Item { Layout.fillWidth: true }

                    FluFilledButton {
                        text: chatManager.imageGen.isGenerating ? "Cancel" : "Generate"
                        enabled: promptInput.text.trim().length > 0 || chatManager.imageGen.isGenerating
                        onClicked: {
                            if (chatManager.imageGen.isGenerating) {
                                chatManager.imageGen.cancel()
                            } else {
                                generatedImage.source = ""
                                revisedPromptText.text = ""
                                errorText.text = ""
                                chatManager.imageGen.generateImage(
                                    promptInput.text.trim(),
                                    sizeCombo.currentText,
                                    qualityCombo.currentText,
                                    styleCombo.currentText
                                )
                            }
                        }
                    }
                }
            }
        }

        // Loading indicator
        FluProgressBar {
            Layout.fillWidth: true
            visible: chatManager.imageGen.isGenerating
            indeterminate: true
        }

        // Error display
        FluText {
            id: errorText
            Layout.fillWidth: true
            color: "#ef4444"
            visible: text.length > 0
            wrapMode: Text.WordWrap
        }

        // Result area
        FluFrame {
            Layout.fillWidth: true
            padding: 16
            visible: generatedImage.source.toString().length > 0 || revisedPromptText.text.length > 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                // Revised prompt
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: revisedPromptText.text.length > 0

                    FluText {
                        text: "Revised Prompt"
                        fontSize: FluTextStyle.BodyStrong
                    }
                    FluText {
                        id: revisedPromptText
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: FluTheme.dark ? "#aaa" : "#555"
                        fontSize: FluTextStyle.Caption
                    }
                }

                // Generated image
                Rectangle {
                    Layout.fillWidth: true
                    height: generatedImage.status === Image.Ready
                        ? Math.min(generatedImage.implicitHeight * (width / generatedImage.implicitWidth), 600)
                        : 0
                    color: "transparent"
                    visible: generatedImage.source.toString().length > 0

                    Image {
                        id: generatedImage
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        cache: true

                        BusyIndicator {
                            anchors.centerIn: parent
                            running: generatedImage.status === Image.Loading
                        }
                    }
                }

                // Action buttons
                RowLayout {
                    Layout.fillWidth: true
                    visible: generatedImage.status === Image.Ready
                    spacing: 8

                    FluButton {
                        text: "Save Image"
                        iconSource: FluentIcons.Save
                        onClicked: saveDialog.open()
                    }

                    FluButton {
                        text: "Use as Reference"
                        iconSource: FluentIcons.ChatBubbles
                        onClicked: {
                            // Download and add to chat attachments
                            imageDownloader.download(generatedImage.source.toString())
                        }
                    }

                    Item { Layout.fillWidth: true }

                    FluButton {
                        text: "Regenerate"
                        iconSource: FluentIcons.Refresh
                        onClicked: {
                            generatedImage.source = ""
                            revisedPromptText.text = ""
                            chatManager.imageGen.generateImage(
                                promptInput.text.trim(),
                                sizeCombo.currentText,
                                qualityCombo.currentText,
                                styleCombo.currentText
                            )
                        }
                    }
                }
            }
        }

        // History section
        FluText {
            text: "Recent Generations"
            fontSize: FluTextStyle.BodyStrong
            visible: historyModel.count > 0
        }

        GridView {
            id: historyGrid
            Layout.fillWidth: true
            height: Math.ceil(historyModel.count / 3) * (cellWidth + 4)
            cellWidth: (width - 16) / 3
            cellHeight: cellWidth + 4
            model: historyModel
            visible: historyModel.count > 0
            interactive: false

            delegate: Rectangle {
                width: historyGrid.cellWidth - 4
                height: historyGrid.cellHeight - 4
                radius: 6
                color: FluTheme.dark ? "#1a1a1a" : "#f0f0f0"
                clip: true

                Image {
                    anchors.fill: parent
                    source: model.url
                    fillMode: Image.PreserveAspectCrop
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: generatedImage.source = model.url
                }
            }
        }
    }

    // History model
    ListModel { id: historyModel }

    // Save dialog
    Platform.FileDialog {
        id: saveDialog
        title: "Save Image"
        fileMode: Platform.FileDialog.SaveFile
        nameFilters: ["PNG Images (*.png)", "JPEG Images (*.jpg)"]
        onAccepted: {
            imageDownloader.downloadAndSave(generatedImage.source.toString(),
                                            file.toString().replace("file:///", "/").replace("file://", ""))
        }
    }

    // Image downloader helper
    QtObject {
        id: imageDownloader
        function download(url) {
            // For now just copy URL to clipboard as a placeholder
            // Full implementation would use QNetworkAccessManager
        }
        function downloadAndSave(url, path) {
            // Placeholder - would download and save
        }
    }

    // Connect signals
    Connections {
        target: chatManager.imageGen

        function onImageReady(imageUrl, revisedPrompt) {
            generatedImage.source = imageUrl
            revisedPromptText.text = revisedPrompt
            errorText.text = ""
            // Add to history
            historyModel.insert(0, { url: imageUrl, prompt: promptInput.text })
            if (historyModel.count > 12) historyModel.remove(12)
        }

        function onErrorOccurred(error) {
            errorText.text = "Error: " + error
        }
    }
}
