import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string content: ""
    property bool isActive: false

    radius: theme.radiusSmall
    color: theme.thinkingBg
    border.color: Qt.rgba(theme.primary.r, theme.primary.g, theme.primary.b, 0.3)
    border.width: 1
    height: thinkLayout.height + 20
    visible: content.length > 0

    ColumnLayout {
        id: thinkLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        spacing: 6

        RowLayout {
            spacing: 6

            Text {
                text: "🧠"
                font.pixelSize: 14
            }

            Text {
                text: isActive ? "Thinking..." : "Thought process"
                color: theme.primary
                font.pixelSize: 12
                font.bold: true
            }

            // Animated dots when active
            Text {
                text: "..."
                color: theme.primary
                font.pixelSize: 12
                visible: isActive

                SequentialAnimation on opacity {
                    running: isActive
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }
            }

            Item { Layout.fillWidth: true }

            // Collapse button
            Text {
                id: collapseBtn
                property bool collapsed: !isActive
                text: collapsed ? "▸ Show" : "▾ Hide"
                color: theme.textSecondary
                font.pixelSize: 11
                visible: !isActive

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: collapseBtn.collapsed = !collapseBtn.collapsed
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: content
            color: theme.textSecondary
            font.pixelSize: 12
            font.italic: true
            wrapMode: Text.Wrap
            lineHeight: 1.4
            visible: isActive || !collapseBtn.collapsed
            maximumLineCount: isActive ? -1 : 50
            elide: Text.ElideRight
        }
    }
}
