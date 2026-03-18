import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: settingsDialog
    anchors.centerIn: parent
    width: Math.min(parent.width - 80, 700)
    height: Math.min(parent.height - 80, 600)
    modal: true
    padding: 0

    background: Rectangle {
        radius: theme.radius
        color: theme.surface
        border.color: theme.border
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16

                Text {
                    text: "Settings"
                    color: theme.text
                    font.pixelSize: 20
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "✕"
                    color: theme.textSecondary
                    font.pixelSize: 20
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: settingsDialog.close()
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: theme.border
            }
        }

        // Tabs
        TabBar {
            id: settingsTab
            Layout.fillWidth: true
            background: Rectangle { color: "transparent" }

            TabButton {
                text: "Providers"
                font.pixelSize: 13
                width: implicitWidth
            }
            TabButton {
                text: "Chat"
                font.pixelSize: 13
                width: implicitWidth
            }
            TabButton {
                text: "Appearance"
                font.pixelSize: 13
                width: implicitWidth
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: theme.border
        }

        // Content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsTab.currentIndex

            // Providers tab
            ScrollView {
                ColumnLayout {
                    width: parent.width - 40
                    x: 20
                    spacing: 16

                    Item { height: 4 }

                    Repeater {
                        model: ["OpenAI", "Claude", "DeepSeek", "Gemini", "Ollama"]
                        delegate: ProviderConfig {
                            Layout.fillWidth: true
                            providerName: modelData
                        }
                    }

                    Item { height: 4 }
                }
            }

            // Chat tab
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    Item { height: 20 }

                    // System prompt
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 8

                        Text {
                            text: "System Prompt"
                            color: theme.text
                            font.pixelSize: 13
                            font.bold: true
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 120

                            TextArea {
                                text: chatManager.settings.systemPrompt
                                placeholderText: "Enter a system prompt to set the AI's behavior..."
                                placeholderTextColor: theme.textSecondary
                                color: theme.text
                                font.pixelSize: 13
                                wrapMode: TextEdit.Wrap
                                onTextChanged: chatManager.settings.systemPrompt = text
                                background: Rectangle {
                                    radius: theme.radiusSmall
                                    color: theme.inputBg
                                    border.color: theme.border
                                }
                                padding: 10
                            }
                        }
                    }

                    // Temperature
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 8

                        RowLayout {
                            Text {
                                text: "Temperature"
                                color: theme.text
                                font.pixelSize: 13
                                font.bold: true
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: tempSlider.value.toFixed(2)
                                color: theme.textSecondary
                                font.pixelSize: 13
                            }
                        }

                        Slider {
                            id: tempSlider
                            Layout.fillWidth: true
                            from: 0; to: 2; stepSize: 0.05
                            value: chatManager.settings.temperature
                            onMoved: chatManager.settings.temperature = value
                        }
                    }

                    // Max tokens
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 8

                        Text {
                            text: "Max Tokens"
                            color: theme.text
                            font.pixelSize: 13
                            font.bold: true
                        }

                        TextField {
                            Layout.fillWidth: true
                            text: chatManager.settings.maxTokens
                            color: theme.text
                            font.pixelSize: 13
                            validator: IntValidator { bottom: 1; top: 200000 }
                            onTextChanged: {
                                var val = parseInt(text)
                                if (!isNaN(val)) chatManager.settings.maxTokens = val
                            }
                            background: Rectangle {
                                radius: theme.radiusSmall
                                color: theme.inputBg
                                border.color: theme.border
                            }
                            padding: 10
                        }
                    }

                    // Send on Enter
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20

                        Text {
                            text: "Send on Enter"
                            color: theme.text
                            font.pixelSize: 13
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Switch {
                            checked: chatManager.settings.sendOnEnter
                            onCheckedChanged: chatManager.settings.sendOnEnter = checked
                        }
                    }

                    Item { height: 20 }
                }
            }

            // Appearance tab
            ScrollView {
                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    Item { height: 20 }

                    // Theme
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20

                        Text {
                            text: "Theme"
                            color: theme.text
                            font.pixelSize: 13
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }

                        Row {
                            spacing: 8

                            Rectangle {
                                width: 80; height: 36
                                radius: theme.radiusSmall
                                color: theme.isDark ? theme.surfaceVariant : theme.primary
                                border.color: theme.isDark ? theme.border : theme.primary

                                Text {
                                    anchors.centerIn: parent
                                    text: "☀ Light"
                                    color: theme.isDark ? theme.text : "white"
                                    font.pixelSize: 12
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: chatManager.settings.theme = "light"
                                }
                            }

                            Rectangle {
                                width: 80; height: 36
                                radius: theme.radiusSmall
                                color: theme.isDark ? theme.primary : theme.surfaceVariant
                                border.color: theme.isDark ? theme.primary : theme.border

                                Text {
                                    anchors.centerIn: parent
                                    text: "🌙 Dark"
                                    color: theme.isDark ? "white" : theme.text
                                    font.pixelSize: 12
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: chatManager.settings.theme = "dark"
                                }
                            }
                        }
                    }

                    // Font size
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 20
                        Layout.rightMargin: 20
                        spacing: 8

                        RowLayout {
                            Text {
                                text: "Font Size"
                                color: theme.text
                                font.pixelSize: 13
                                font.bold: true
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: fontSlider.value.toFixed(0) + "px"
                                color: theme.textSecondary
                                font.pixelSize: 13
                            }
                        }

                        Slider {
                            id: fontSlider
                            Layout.fillWidth: true
                            from: 10; to: 24; stepSize: 1
                            value: chatManager.settings.fontSize
                            onMoved: chatManager.settings.fontSize = value
                        }
                    }

                    Item { height: 20 }
                }
            }
        }
    }
}
