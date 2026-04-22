import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluScrollablePage {
    id: settingsPage
    title: "Settings"

    ColumnLayout {
        width: Math.min(parent.width - 48, 720)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        FluPivot {
            id: pivot
            Layout.fillWidth: true

            FluPivotItem {
                title: "General"
                contentItem: ColumnLayout {
                    spacing: 16

                    Item { height: 8 }

                    // Send mode
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        RowLayout {
                            anchors.fill: parent
                            spacing: 12

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                FluText { text: "Send Mode"; font: FluTextStyle.BodyStrong }
                                FluText {
                                    text: "How to send messages"
                                    color: FluTheme.dark ? "#aaa" : "#666"
                                    font: FluTextStyle.Caption
                                }
                            }

                            FluComboBox {
                                model: ["Enter to send", "Ctrl+Enter to send"]
                                currentIndex: chatManager.settings.sendOnEnter ? 0 : 1
                                implicitWidth: 180
                                onCurrentIndexChanged: chatManager.settings.sendOnEnter = (currentIndex === 0)
                            }
                        }
                    }

                    // Math renderer
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        RowLayout {
                            anchors.fill: parent
                            FluText { text: "Math Renderer"; font: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                            FluToggleSwitch {
                                checked: chatManager.settings.mathRenderer
                                onCheckedChanged: chatManager.settings.mathRenderer = checked
                            }
                        }
                    }

                    // Mermaid diagrams
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        RowLayout {
                            anchors.fill: parent
                            FluText { text: "Mermaid Diagrams"; font: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                            FluToggleSwitch {
                                checked: chatManager.settings.mermaidEnabled
                                onCheckedChanged: chatManager.settings.mermaidEnabled = checked
                            }
                        }
                    }

                    // Markdown rendering
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        RowLayout {
                            anchors.fill: parent
                            FluText { text: "Markdown Rendering"; font: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                            FluToggleSwitch {
                                checked: chatManager.settings.markdownRendering
                                onCheckedChanged: chatManager.settings.markdownRendering = checked
                            }
                        }
                    }

                    // Temperature
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                FluText { text: "Temperature"; font: FluTextStyle.BodyStrong; Layout.fillWidth: true }
                                FluText {
                                    text: tempSlider.value.toFixed(2)
                                    color: FluTheme.dark ? "#aaa" : "#666"
                                    font: FluTextStyle.Caption
                                }
                            }

                            FluSlider {
                                id: tempSlider
                                Layout.fillWidth: true
                                from: 0; to: 2; stepSize: 0.05
                                value: chatManager.settings.temperature
                                onMoved: chatManager.settings.temperature = value
                            }
                        }
                    }

                    // History message count
                    FluFrame {
                        Layout.fillWidth: true
                        padding: 16

                        RowLayout {
                            anchors.fill: parent
                            spacing: 12

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                FluText { text: "History Message Count"; font: FluTextStyle.BodyStrong }
                                FluText {
                                    text: "Number of past messages sent to the model"
                                    color: FluTheme.dark ? "#aaa" : "#666"
                                    font: FluTextStyle.Caption
                                }
                            }

                            FluTextBox {
                                implicitWidth: 80
                                text: chatManager.settings.historyMessageCount
                                validator: IntValidator { bottom: 1; top: 200 }
                                onEditingFinished: {
                                    var v = parseInt(text)
                                    if (!isNaN(v)) chatManager.settings.historyMessageCount = v
                                }
                            }
                        }
                    }

                    Item { height: 8 }
                }
            }

            FluPivotItem {
                title: "Providers"
                contentItem: ColumnLayout {
                    spacing: 12

                    Item { height: 8 }

                    Repeater {
                        model: chatManager.providerNames()
                        delegate: ProviderConfigPanel {
                            Layout.fillWidth: true
                            providerName: modelData
                        }
                    }

                    Item { height: 8 }
                }
            }
        }
    }
}
