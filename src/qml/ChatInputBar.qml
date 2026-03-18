import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform
import FluentUI

Rectangle {
    id: inputBar
    width: parent.width
    height: inputColumn.implicitHeight + 16
    color: FluTheme.dark ? "#1f1f1f" : "#f9f9f9"

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: FluTheme.dark ? "#333" : "#e0e0e0"
    }

    ColumnLayout {
        id: inputColumn
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 8
            topMargin: 12
        }
        spacing: 8

        // Attached files chips
        Flow {
            Layout.fillWidth: true
            spacing: 6
            visible: attachedFiles.count > 0

            Repeater {
                id: attachedFiles
                model: ListModel { id: attachedFilesModel }
                delegate: FluButton {
                    text: model.name
                    iconSource: FluentIcons.Attach
                    onClicked: attachedFilesModel.remove(index)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            // Upload button + menu
            FluIconButton {
                id: attachBtn
                iconSource: FluentIcons.Attach
                iconSize: 18
                ToolTip.text: "Attach file or image"
                ToolTip.visible: hovered
                onClicked: uploadMenu.popup()
            }

            FluMenu {
                id: uploadMenu
                FluMenuItem {
                    text: "Upload File"
                    iconSource: FluentIcons.Document
                    onClicked: fileDialog.open()
                }
                FluMenuItem {
                    text: "Upload Image"
                    iconSource: FluentIcons.Photo
                    onClicked: imageDialog.open()
                }
            }

            // Mode selector
            FluComboBox {
                id: modeCombo
                model: ["Quick", "Think", "Expert"]
                currentIndex: {
                    var m = chatManager.settings.chatMode
                    if (m === "think") return 1
                    if (m === "expert") return 2
                    return 0
                }
                implicitWidth: 100
                onCurrentIndexChanged: {
                    var modes = ["quick", "think", "expert"]
                    chatManager.settings.chatMode = modes[currentIndex]
                }
            }

            // Deep Research toggle
            FluToggleSwitch {
                id: deepResearchToggle
                text: "Deep Research"
                checked: chatManager.settings.deepResearch
                onCheckedChanged: chatManager.settings.deepResearch = checked
            }

            // Text input
            FluMultilineTextBox {
                id: inputArea
                Layout.fillWidth: true
                placeholderText: "Type a message... (Enter to send, Shift+Enter for new line)"
                implicitHeight: Math.min(contentHeight + 24, 120)

                Keys.onReturnPressed: (event) => {
                    if (event.modifiers & Qt.ShiftModifier) {
                        inputArea.insert(inputArea.cursorPosition, "\n")
                    } else if (chatManager.settings.sendOnEnter) {
                        sendCurrentMessage()
                        event.accepted = true
                    }
                }
                Keys.onEnterPressed: (event) => {
                    if (!(event.modifiers & Qt.ShiftModifier) && chatManager.settings.sendOnEnter) {
                        sendCurrentMessage()
                        event.accepted = true
                    }
                }
            }

            // Clear context
            FluIconButton {
                iconSource: FluentIcons.Delete
                iconSize: 18
                ToolTip.text: "Clear context"
                ToolTip.visible: hovered
                onClicked: chatManager.clearCurrentConversation()
            }

            // Send / Stop button
            FluFilledButton {
                text: chatManager.isGenerating ? "Stop" : "Send"
                iconSource: chatManager.isGenerating ? FluentIcons.Stop : FluentIcons.Send
                onClicked: {
                    if (chatManager.isGenerating) {
                        chatManager.stopGeneration()
                    } else {
                        sendCurrentMessage()
                    }
                }
            }
        }
    }

    // File dialog
    Platform.FileDialog {
        id: fileDialog
        title: "Attach File"
        nameFilters: [
            "Supported files (*.txt *.md *.csv *.json *.xml *.html *.py *.js *.ts *.cpp *.c *.h *.java *.rs *.go *.rb *.php *.yaml *.yml *.toml *.sql)",
            "All files (*)"
        ]
        onAccepted: {
            var path = file.toString().replace("file:///", "/").replace("file://", "")
            if (chatManager.documentParser.isSupported(path)) {
                attachedFilesModel.append({
                    "name": chatManager.documentParser.fileDescription(path),
                    "path": path
                })
            }
        }
    }

    Platform.FileDialog {
        id: imageDialog
        title: "Attach Image"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.gif *.webp)", "All files (*)"]
        onAccepted: {
            var path = file.toString().replace("file:///", "/").replace("file://", "")
            var name = path.split("/").pop()
            attachedFilesModel.append({"name": name, "path": path})
        }
    }

    function sendCurrentMessage() {
        var text = inputArea.text.trim()
        if (text.length === 0 && attachedFilesModel.count === 0) return

        var attachments = []
        for (var i = 0; i < attachedFilesModel.count; i++) {
            attachments.push(attachedFilesModel.get(i).path)
        }

        chatManager.sendMessage(text, attachments)
        inputArea.clear()
        attachedFilesModel.clear()
    }
}
