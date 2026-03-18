import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string fileName: ""
    signal remove()

    width: chipRow.width + 36
    height: 28
    radius: 14
    color: Qt.rgba(theme.primary.r, theme.primary.g, theme.primary.b, 0.15)
    border.color: Qt.rgba(theme.primary.r, theme.primary.g, theme.primary.b, 0.3)
    border.width: 1

    RowLayout {
        id: chipRow
        anchors.centerIn: parent
        spacing: 4

        Text {
            text: "📎"
            font.pixelSize: 12
        }

        Text {
            text: fileName
            color: theme.primary
            font.pixelSize: 11
            maximumLineCount: 1
            elide: Text.ElideMiddle
            Layout.maximumWidth: 160
        }

        Text {
            text: "✕"
            color: theme.textSecondary
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.remove()
            }
        }
    }
}
