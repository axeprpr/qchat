import QtQuick

Rectangle {
    id: root
    property string text: ""
    property color textColor: theme.primary

    width: tagText.width + 16
    height: 22
    radius: 11
    color: Qt.rgba(textColor.r, textColor.g, textColor.b, 0.12)

    Text {
        id: tagText
        anchors.centerIn: parent
        text: root.text
        color: root.textColor
        font.pixelSize: 11
    }
}
