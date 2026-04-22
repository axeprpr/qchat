import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluContentDialog {
    id: promptDialog
    title: "Prompt Library"
    width: 600
    height: 500

    property string selectedContent: ""

    buttonFlags: FluContentDialogType.NegativeButton | FluContentDialogType.PositiveButton
    negativeText: "Cancel"
    positiveText: "Use Prompt"
    onPositiveClicked: {
        if (promptList.currentIndex >= 0) {
            var idx = filteredModel.get(promptList.currentIndex).originalIndex
            selectedContent = chatManager.promptLibrary.getContent(idx)
        }
    }

    contentDelegate: Component {
        ColumnLayout {
            spacing: 8
            width: parent ? parent.width : 560

            // Search bar
            FluTextBox {
                id: searchBox
                Layout.fillWidth: true
                placeholderText: "Search prompts..."
                onTextChanged: filterTimer.restart()
            }

            Timer {
                id: filterTimer
                interval: 200
                onTriggered: filteredModel.applyFilter(searchBox.text)
            }

            // Prompt list
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                height: 300
                color: FluTheme.dark ? "#1a1a1a" : "#f5f5f5"
                radius: 4
                border.color: FluTheme.dark ? "#333" : "#ddd"
                border.width: 1

                ListView {
                    id: promptList
                    anchors.fill: parent
                    anchors.margins: 4
                    clip: true
                    model: filteredModel
                    ScrollBar.vertical: FluScrollBar {}

                    delegate: Rectangle {
                        width: promptList.width
                        height: 56
                        radius: 4
                        color: promptList.currentIndex === index
                            ? (FluTheme.dark ? "#2d4a7a" : "#dbeafe")
                            : (hovered ? (FluTheme.dark ? "#2a2a2a" : "#ebebeb") : "transparent")

                        property bool hovered: false

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                            onClicked: promptList.currentIndex = index
                            onDoubleClicked: {
                                promptList.currentIndex = index
                                var idx = filteredModel.get(index).originalIndex
                                promptDialog.selectedContent = chatManager.promptLibrary.getContent(idx)
                                promptDialog.close()
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                FluText {
                                    text: model.title
                                    fontSize: FluTextStyle.Body
                                    font.bold: true
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Rectangle {
                                    width: categoryLabel.implicitWidth + 8
                                    height: 18
                                    radius: 9
                                    color: FluTheme.dark ? "#1e3a5f" : "#dbeafe"
                                    FluText {
                                        id: categoryLabel
                                        anchors.centerIn: parent
                                        text: model.category
                                        fontSize: FluTextStyle.Caption
                                        color: FluTheme.dark ? "#93c5fd" : "#1d4ed8"
                                    }
                                }
                            }

                            FluText {
                                text: model.content
                                fontSize: FluTextStyle.Caption
                                color: FluTheme.dark ? "#888" : "#666"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }
                        }
                    }
                }
            }

            // Action buttons row
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                FluButton {
                    text: "New Prompt"
                    iconSource: FluentIcons.Add
                    onClicked: editDialog.openNew()
                }

                Item { Layout.fillWidth: true }

                FluButton {
                    text: "Edit"
                    iconSource: FluentIcons.Edit
                    enabled: promptList.currentIndex >= 0
                    onClicked: {
                        var idx = filteredModel.get(promptList.currentIndex).originalIndex
                        editDialog.openEdit(idx)
                    }
                }

                FluButton {
                    text: "Delete"
                    iconSource: FluentIcons.Delete
                    enabled: promptList.currentIndex >= 0
                    onClicked: {
                        var idx = filteredModel.get(promptList.currentIndex).originalIndex
                        chatManager.promptLibrary.deletePrompt(idx)
                        filteredModel.applyFilter(searchBox.text)
                        promptList.currentIndex = -1
                    }
                }
            }
        }
    }

    // Filtered list model
    ListModel {
        id: filteredModel

        function applyFilter(query) {
            clear()
            var all = chatManager.promptLibrary.prompts
            for (var i = 0; i < all.length; i++) {
                var p = all[i]
                var q = query.toLowerCase()
                if (!query || p.title.toLowerCase().includes(q) ||
                    p.content.toLowerCase().includes(q) ||
                    p.category.toLowerCase().includes(q)) {
                    append({
                        title: p.title,
                        content: p.content,
                        category: p.category,
                        originalIndex: i
                    })
                }
            }
        }
    }

    // Edit/New prompt dialog
    FluContentDialog {
        id: editDialog
        title: editDialog.isNew ? "New Prompt" : "Edit Prompt"
        property bool isNew: true
        property int editIndex: -1

        buttonFlags: FluContentDialogType.NegativeButton | FluContentDialogType.PositiveButton
        negativeText: "Cancel"
        positiveText: "Save"

        onPositiveClicked: {
            if (editTitle.text.trim().length === 0) return
            if (isNew) {
                chatManager.promptLibrary.addPrompt(editTitle.text.trim(), editContent.text, editCategory.text.trim() || "General")
            } else {
                chatManager.promptLibrary.updatePrompt(editIndex, editTitle.text.trim(), editContent.text, editCategory.text.trim() || "General")
            }
            filteredModel.applyFilter(searchBox.text)
        }

        contentDelegate: Component {
            ColumnLayout {
                spacing: 8
                width: parent ? parent.width : 400

                FluTextBox {
                    id: editTitle
                    Layout.fillWidth: true
                    placeholderText: "Title"
                }
                FluTextBox {
                    id: editCategory
                    Layout.fillWidth: true
                    placeholderText: "Category (e.g. Development, Writing)"
                }
                FluMultilineTextBox {
                    id: editContent
                    Layout.fillWidth: true
                    implicitHeight: 120
                    placeholderText: "Prompt content..."
                }
            }
        }

        function openNew() {
            isNew = true
            editIndex = -1
            open()
        }

        function openEdit(idx) {
            isNew = false
            editIndex = idx
            var p = chatManager.promptLibrary.prompts[idx]
            open()
        }
    }

    Component.onCompleted: {
        filteredModel.applyFilter("")
    }

    Connections {
        target: chatManager.promptLibrary
        function onPromptsChanged() {
            filteredModel.applyFilter(searchBox ? searchBox.text : "")
        }
    }
}
