import QtQuick
import QtQuick.Controls

Button {
    id: root
    width: 36
    height: 36
    flat: true
    icon.width: 18
    icon.height: 18
    icon.color: hovered ? theme.text : theme.textSecondary
    padding: 0

    background: Rectangle {
        radius: theme.radiusSmall
        color: root.pressed ? Qt.rgba(theme.primary.r, theme.primary.g, theme.primary.b, 0.2) :
               root.hovered ? Qt.rgba(theme.text.r, theme.text.g, theme.text.b, 0.08) : "transparent"
    }
}
