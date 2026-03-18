import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Qt.labs.platform as Platform

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "QChat"
    color: theme.background

    // Theme
    QtObject {
        id: theme
        property bool isDark: chatManager.settings.theme === "dark"

        property color background: isDark ? "#1a1a2e" : "#f5f5f5"
        property color surface: isDark ? "#16213e" : "#ffffff"
        property color surfaceVariant: isDark ? "#1a2744" : "#f0f0f0"
        property color primary: "#4361ee"
        property color primaryHover: "#3a56d4"
        property color onPrimary: "#ffffff"
        property color text: isDark ? "#e6e6e6" : "#1a1a1a"
        property color textSecondary: isDark ? "#a0a0b0" : "#666666"
        property color border: isDark ? "#2a3a5c" : "#e0e0e0"
        property color danger: "#e74c3c"
        property color success: "#2ecc71"
        property color warning: "#f39c12"
        property color userBubble: "#4361ee"
        property color assistantBubble: isDark ? "#1e2d4a" : "#f0f0f5"
        property color thinkingBg: isDark ? "#1a2335" : "#f8f8ff"
        property color inputBg: isDark ? "#0f1a30" : "#ffffff"
        property color sidebarBg: isDark ? "#0f1529" : "#fafafa"
        property color scrollbar: isDark ? "#3a4a6c" : "#cccccc"
        property color errorBg: isDark ? "#2d1a1a" : "#fff0f0"

        property int radius: 12
        property int radiusSmall: 8
        property int spacing: 12
        property int fontSizeBase: chatManager.settings.fontSize
    }

    // Shortcut
    Shortcut { sequence: "Ctrl+N"; onActivated: chatManager.newConversation() }
    Shortcut { sequence: "Ctrl+,"; onActivated: settingsDialog.open() }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar
        Sidebar {
            id: sidebar
            Layout.preferredWidth: sidebarVisible ? 280 : 0
            Layout.fillHeight: true
            property bool sidebarVisible: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
        }

        // Separator
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: theme.border
            visible: sidebar.sidebarVisible
        }

        // Main content
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Top bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: theme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    // Toggle sidebar
                    IconButton {
                        icon.source: "qrc:/icons/sidebar.svg"
                        onClicked: sidebar.sidebarVisible = !sidebar.sidebarVisible
                        ToolTip.text: "Toggle sidebar"
                        ToolTip.visible: hovered
                    }

                    // Model selector
                    ComboBox {
                        id: providerCombo
                        Layout.preferredWidth: 140
                        model: ["OpenAI", "Claude", "DeepSeek", "Gemini", "Ollama"]
                        currentIndex: model.indexOf(chatManager.settings.currentProvider)
                        onCurrentTextChanged: {
                            chatManager.settings.currentProvider = currentText
                            modelCombo.model = chatManager.availableModels()
                        }
                        font.pixelSize: 13

                        background: Rectangle {
                            radius: theme.radiusSmall
                            color: theme.surfaceVariant
                            border.color: providerCombo.hovered ? theme.primary : theme.border
                            border.width: 1
                        }
                        contentItem: Text {
                            text: providerCombo.displayText
                            color: theme.text
                            font: providerCombo.font
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }
                    }

                    ComboBox {
                        id: modelCombo
                        Layout.preferredWidth: 200
                        model: chatManager.availableModels()
                        currentIndex: {
                            var models = chatManager.availableModels()
                            return Math.max(0, models.indexOf(chatManager.settings.currentModel))
                        }
                        onCurrentTextChanged: chatManager.settings.currentModel = currentText
                        font.pixelSize: 13
                        editable: true

                        background: Rectangle {
                            radius: theme.radiusSmall
                            color: theme.surfaceVariant
                            border.color: modelCombo.hovered ? theme.primary : theme.border
                            border.width: 1
                        }
                        contentItem: TextField {
                            text: modelCombo.editText
                            color: theme.text
                            font: modelCombo.font
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                            background: null
                            onAccepted: chatManager.settings.currentModel = text
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Deep Thinking toggle
                    Rectangle {
                        Layout.preferredWidth: thinkRow.width + 20
                        Layout.preferredHeight: 34
                        radius: 17
                        color: chatManager.settings.enableThinking ? theme.primary : theme.surfaceVariant
                        border.color: chatManager.settings.enableThinking ? theme.primary : theme.border
                        border.width: 1
                        cursor: Qt.PointingHandCursor

                        RowLayout {
                            id: thinkRow
                            anchors.centerIn: parent
                            spacing: 6

                            Text {
                                text: "🧠"
                                font.pixelSize: 14
                            }
                            Text {
                                text: "Deep Think"
                                color: chatManager.settings.enableThinking ? theme.onPrimary : theme.text
                                font.pixelSize: 12
                                font.bold: chatManager.settings.enableThinking
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: chatManager.settings.enableThinking = !chatManager.settings.enableThinking
                        }
                    }

                    // Theme toggle
                    IconButton {
                        icon.source: theme.isDark ? "qrc:/icons/sun.svg" : "qrc:/icons/moon.svg"
                        onClicked: chatManager.settings.theme = theme.isDark ? "light" : "dark"
                        ToolTip.text: theme.isDark ? "Light mode" : "Dark mode"
                        ToolTip.visible: hovered
                    }

                    // Settings
                    IconButton {
                        icon.source: "qrc:/icons/settings.svg"
                        onClicked: settingsDialog.open()
                        ToolTip.text: "Settings (Ctrl+,)"
                        ToolTip.visible: hovered
                    }
                }

                // Bottom border
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: theme.border
                }
            }

            // Chat area
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ChatView {
                    id: chatView
                    anchors.fill: parent
                    visible: chatManager.messageModel.count > 0
                }

                WelcomeView {
                    anchors.fill: parent
                    visible: chatManager.messageModel.count === 0
                }
            }

            // Input area
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(inputArea.contentHeight + 40, 200)
                Layout.minimumHeight: 60
                color: theme.surface

                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: 1
                    color: theme.border
                }

                // Attached files chips
                Flow {
                    id: attachedFiles
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 8
                    spacing: 6
                    visible: attachedFilesList.count > 0

                    Repeater {
                        model: ListModel { id: attachedFilesList }
                        delegate: DocumentChip {
                            fileName: model.name
                            onRemove: attachedFilesList.remove(index)
                        }
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    anchors.topMargin: attachedFiles.visible ? attachedFiles.height + 14 : 10
                    spacing: 8

                    // Attach button
                    IconButton {
                        icon.source: "qrc:/icons/attach.svg"
                        onClicked: fileDialog.open()
                        ToolTip.text: "Attach file"
                        ToolTip.visible: hovered
                    }

                    // Text input
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        TextArea {
                            id: inputArea
                            placeholderText: "Type your message... (Enter to send, Shift+Enter for new line)"
                            placeholderTextColor: theme.textSecondary
                            color: theme.text
                            font.pixelSize: theme.fontSizeBase
                            wrapMode: TextEdit.Wrap
                            background: Rectangle {
                                radius: theme.radius
                                color: theme.inputBg
                                border.color: inputArea.activeFocus ? theme.primary : theme.border
                                border.width: inputArea.activeFocus ? 2 : 1
                            }
                            padding: 12

                            Keys.onReturnPressed: (event) => {
                                if (event.modifiers & Qt.ShiftModifier) {
                                    inputArea.insert(inputArea.cursorPosition, "\n")
                                } else {
                                    sendCurrentMessage()
                                    event.accepted = true
                                }
                            }
                        }
                    }

                    // Send / Stop button
                    Rectangle {
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 44
                        Layout.alignment: Qt.AlignBottom
                        radius: theme.radius
                        color: chatManager.isGenerating ? theme.danger :
                               (inputArea.text.trim().length > 0 ? theme.primary : theme.surfaceVariant)

                        Behavior on color { ColorAnimation { duration: 200 } }

                        Image {
                            anchors.centerIn: parent
                            source: chatManager.isGenerating ? "qrc:/icons/stop.svg" : "qrc:/icons/send.svg"
                            width: 20; height: 20
                            sourceSize: Qt.size(20, 20)
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
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
            }
        }
    }

    SettingsDialog {
        id: settingsDialog
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Attach File"
        nameFilters: ["All supported (*.txt *.md *.csv *.json *.xml *.html *.py *.js *.ts *.cpp *.c *.h *.java *.rs *.go *.rb *.php *.yaml *.yml *.toml *.sql)",
                      "Text files (*.txt *.md *.csv)", "Code files (*.py *.js *.ts *.cpp *.java *.rs *.go)",
                      "Data files (*.json *.xml *.yaml *.yml)", "All files (*)"]
        onAccepted: {
            var path = file.toString().replace("file:///", "/").replace("file://", "")
            if (chatManager.documentParser.isSupported(path)) {
                attachedFilesList.append({"name": chatManager.documentParser.fileDescription(path), "path": path})
            }
        }
    }

    function sendCurrentMessage() {
        var text = inputArea.text.trim()
        if (text.length === 0 && attachedFilesList.count === 0) return

        var attachments = []
        for (var i = 0; i < attachedFilesList.count; i++) {
            attachments.push(attachedFilesList.get(i).path)
        }

        chatManager.sendMessage(text, attachments)
        inputArea.clear()
        attachedFilesList.clear()
    }

    // Error notification
    Connections {
        target: chatManager
        function onError(message) {
            errorPopup.text = message
            errorPopup.open()
        }
    }

    Popup {
        id: errorPopup
        property string text: ""
        x: (parent.width - width) / 2
        y: 20
        width: Math.min(parent.width - 40, 500)
        padding: 16
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: theme.radius
            color: theme.errorBg
            border.color: theme.danger
            border.width: 1
        }

        contentItem: Text {
            text: errorPopup.text
            color: theme.danger
            font.pixelSize: 13
            wrapMode: Text.Wrap
        }

        Timer {
            running: errorPopup.visible
            interval: 5000
            onTriggered: errorPopup.close()
        }
    }
}
