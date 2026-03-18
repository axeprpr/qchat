import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluContentDialog {
    id: root
    title: "Tool Settings"
    positiveText: "Save"
    negativeText: "Cancel"

    onPositiveClicked: {
        chatManager.settings.mathRenderer = mathSwitch.checked
        chatManager.settings.mermaidEnabled = mermaidSwitch.checked
        chatManager.settings.markdownRendering = markdownSwitch.checked
        FluToast.success("Tool settings saved")
    }

    contentDelegate: Component {
        ColumnLayout {
            width: 400
            spacing: 16

            FluText {
                text: "Configure rendering and parsing tools"
                color: FluTheme.dark ? "#aaa" : "#666"
                fontSize: FluTextStyle.Caption
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            FluFrame {
                Layout.fillWidth: true
                padding: 16

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            FluText { text: "Markdown Rendering"; fontSize: FluTextStyle.BodyStrong }
                            FluText {
                                text: "Render assistant messages as formatted markdown"
                                color: FluTheme.dark ? "#aaa" : "#666"
                                fontSize: FluTextStyle.Caption
                            }
                        }
                        FluToggleSwitch {
                            id: markdownSwitch
                            checked: chatManager.settings.markdownRendering
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: FluTheme.dark ? "#333" : "#e0e0e0" }

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            FluText { text: "Math Renderer"; fontSize: FluTextStyle.BodyStrong }
                            FluText {
                                text: "Render LaTeX math expressions"
                                color: FluTheme.dark ? "#aaa" : "#666"
                                fontSize: FluTextStyle.Caption
                            }
                        }
                        FluToggleSwitch {
                            id: mathSwitch
                            checked: chatManager.settings.mathRenderer
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: FluTheme.dark ? "#333" : "#e0e0e0" }

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            spacing: 2
                            Layout.fillWidth: true
                            FluText { text: "Mermaid Diagrams"; fontSize: FluTextStyle.BodyStrong }
                            FluText {
                                text: "Render Mermaid diagram code blocks"
                                color: FluTheme.dark ? "#aaa" : "#666"
                                fontSize: FluTextStyle.Caption
                            }
                        }
                        FluToggleSwitch {
                            id: mermaidSwitch
                            checked: chatManager.settings.mermaidEnabled
                        }
                    }
                }
            }
        }
    }
}
