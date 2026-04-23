import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

Rectangle {
    id: root
    property string content: ""
    property bool isActive: false

    width: parent.width
    height: expanded ? contentCol.implicitHeight + 16 : headerRow.implicitHeight + 16
    radius: 6
    color: FluTheme.dark ? "#1a2335" : "#f0f4ff"
    border.color: FluTheme.dark ? "#2a3a5c" : "#c8d8ff"
    border.width: 1
    clip: true

    property bool expanded: false

    Behavior on height {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    ColumnLayout {
        id: contentCol
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
        spacing: 6

        RowLayout {
            id: headerRow
            Layout.fillWidth: true
            spacing: 6

            FluProgressRing {
                width: 16; height: 16
                visible: isActive
            }

            FluIcon {
                iconSource: FluentIcons.Processing
                iconSize: 14
                visible: !isActive
                color: FluTheme.dark ? "#7090d0" : "#4060c0"
            }

            FluText {
                text: isActive ? "Thinking..." : "Thought process"
                color: FluTheme.dark ? "#7090d0" : "#4060c0"
                font: FluTextStyle.Caption
                Layout.fillWidth: true
            }

            FluIconButton {
                iconSource: expanded ? FluentIcons.ChevronUp : FluentIcons.ChevronDown
                iconSize: 12
                visible: !isActive && content.length > 0
                onClicked: expanded = !expanded
            }
        }

        FluText {
            Layout.fillWidth: true
            text: content
            color: FluTheme.dark ? "#8090a8" : "#506080"
            font: FluTextStyle.Caption
            wrapMode: Text.Wrap
            visible: expanded && content.length > 0
        }
    }
}
