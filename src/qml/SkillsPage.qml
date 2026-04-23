import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluPage {
    id: root
    title: "Skills"

    property var skillsData: []
    property string selectedSkillId: ""

    function currentSkill() {
        if (!selectedSkillId)
            return ({})
        return chatManager.skillById(selectedSkillId)
    }

    function reloadSkills() {
        skillsData = chatManager.skills()
        if (skillsData.length === 0) {
            selectedSkillId = ""
            clearForm()
            return
        }

        var found = false
        for (var i = 0; i < skillsData.length; i++) {
            if (skillsData[i].id === selectedSkillId) {
                found = true
                break
            }
        }
        if (!found) {
            selectedSkillId = skillsData[0].id
        }
        loadForm(currentSkill())
    }

    function clearForm() {
        idField.text = ""
        nameField.text = ""
        descriptionField.text = ""
        promptField.text = ""
        readonlyHint.visible = false
    }

    function loadForm(skill) {
        idField.text = skill.id || ""
        nameField.text = skill.name || ""
        descriptionField.text = skill.description || ""
        promptField.text = skill.prompt || ""
        readonlyHint.visible = skill.readonly === true
    }

    function collectForm() {
        var current = currentSkill()
        return {
            "id": idField.text,
            "name": nameField.text,
            "description": descriptionField.text,
            "prompt": promptField.text,
            "readonly": current.readonly === true,
            "builtin": current.builtin === true
        }
    }

    Component.onCompleted: reloadSkills()

    Connections {
        target: chatManager
        function onSkillsChanged() {
            reloadSkills()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        FluFrame {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            padding: 12

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: "Skill List"
                        font: FluTextStyle.Subtitle
                        Layout.fillWidth: true
                    }
                    FluIconButton {
                        iconSource: FluentIcons.Add
                        ToolTip.text: "New Skill"
                        ToolTip.visible: hovered
                        onClicked: {
                            selectedSkillId = ""
                            clearForm()
                            nameField.text = "New Skill"
                        }
                    }
                }

                ListView {
                    id: skillListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: skillsData
                    spacing: 4

                    delegate: Rectangle {
                        width: skillListView.width
                        height: 44
                        radius: 8
                        color: modelData.id === selectedSkillId
                               ? (FluTheme.dark ? "#2a3b57" : "#dbeafe")
                               : (FluTheme.dark ? "#262626" : "#f5f5f5")

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            FluText {
                                text: modelData.name || modelData.id
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                selectedSkillId = modelData.id
                                loadForm(chatManager.skillById(selectedSkillId))
                            }
                        }
                    }
                }
            }
        }

        FluFrame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 16

            ScrollView {
                anchors.fill: parent
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    FluText {
                        text: "Skill Config"
                        font: FluTextStyle.Subtitle
                    }

                    FluText {
                        id: readonlyHint
                        visible: false
                        text: "Builtin skill: delete is disabled"
                        color: FluTheme.dark ? "#f59e0b" : "#b45309"
                        font: FluTextStyle.Caption
                    }

                    FluTextBox {
                        id: idField
                        Layout.fillWidth: true
                        visible: false
                    }

                    FluTextBox {
                        id: nameField
                        Layout.fillWidth: true
                        placeholderText: "Skill name"
                    }

                    FluMultilineTextBox {
                        id: descriptionField
                        Layout.fillWidth: true
                        implicitHeight: 70
                        placeholderText: "Description"
                    }

                    FluMultilineTextBox {
                        id: promptField
                        Layout.fillWidth: true
                        implicitHeight: 220
                        placeholderText: "Skill prompt / behavior contract"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        FluFilledButton {
                            text: "Save Skill"
                            onClicked: {
                                var payload = collectForm()
                                if (!payload.name || payload.name.trim() === "") {
                                    FluToast.error("Skill name is required")
                                    return
                                }
                                var wasNew = !payload.id || payload.id === ""
                                chatManager.saveSkill(payload)
                                reloadSkills()
                                if (wasNew && skillsData.length > 0) {
                                    selectedSkillId = skillsData[skillsData.length - 1].id
                                    loadForm(chatManager.skillById(selectedSkillId))
                                }
                                FluToast.success("Skill saved")
                            }
                        }

                        FluButton {
                            text: "Delete"
                            enabled: !currentSkill().readonly
                            onClicked: {
                                if (!selectedSkillId)
                                    return
                                chatManager.deleteSkill(selectedSkillId)
                                selectedSkillId = ""
                                reloadSkills()
                                FluToast.success("Skill deleted")
                            }
                        }
                    }
                }
            }
        }
    }
}
