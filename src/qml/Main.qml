import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI

FluWindow {
    id: window
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    title: "QChat"
    fitsAppBarWindows: true

    appBar: FluAppBar {
        id: appBar
        title: "QChat"
        showDark: true
        darkClickListener: function() {
            FluTheme.darkMode = FluTheme.darkMode === FluThemeType.Dark
                ? FluThemeType.Light : FluThemeType.Dark
        }
        z: 7
    }

    Shortcut { sequence: "Ctrl+N"; onActivated: chatManager.newConversation() }

    FluNavigationView {
        id: nav
        anchors.fill: parent
        pageMode: FluNavigationViewType.NoStack
        displayMode: FluNavigationViewType.Open

        logo: Image {
            source: "qrc:/icons/logo.svg"
            width: 28
            height: 28
            sourceSize: Qt.size(28, 28)
        }

        title: "QChat"

        Component.onCompleted: {
            nav.setCurrentIndex(0)
        }

        items: ObjectModel {
            FluPaneItemHeader { title: "Chats" }

            FluPaneItem {
                id: newChatItem
                title: "New Chat"
                icon: FluentIcons.Add
                onTap: {
                    chatManager.newConversation()
                }
            }

            FluPaneItemSeparator {}

            Repeater {
                model: chatManager.conversationModel
                delegate: FluPaneItem {
                    title: model.title || "New Chat"
                    icon: FluentIcons.Chat
                    onTap: {
                        chatManager.switchConversation(index)
                        nav.push("qrc:/qt/qml/QChat/src/qml/ChatPage.qml")
                    }
                }
            }

            FluPaneItemSeparator {}

            FluPaneItem {
                title: "Calls"
                icon: FluentIcons.Phone
                onTap: {
                    nav.push("qrc:/qt/qml/QChat/src/qml/CallsPage.qml")
                }
            }

            FluPaneItem {
                title: "Image Gen"
                icon: FluentIcons.Photo
                onTap: {
                    nav.push("qrc:/qt/qml/QChat/src/qml/ImageGenPage.qml")
                }
            }
        }

        footerItems: ObjectModel {
            FluPaneItem {
                title: "Settings"
                icon: FluentIcons.Settings
                onTap: {
                    nav.push("qrc:/qt/qml/QChat/src/qml/SettingsPage.qml")
                }
            }
        }

        // Default page
        FluFrame {
            anchors.fill: parent

            Loader {
                id: pageLoader
                anchors.fill: parent
                source: chatManager.messageModel.count > 0
                    ? "qrc:/qt/qml/QChat/src/qml/ChatPage.qml"
                    : "qrc:/qt/qml/QChat/src/qml/WelcomePage.qml"
            }
        }
    }

    // Error toast
    Connections {
        target: chatManager
        function onError(message) {
            FluToast.error(message)
        }
    }
}
