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

    Shortcut { sequence: "Ctrl+N"; onActivated: newConversationDialog.open() }

    FluNavigationView {
        id: nav
        anchors.fill: parent
        pageMode: FluNavigationViewType.NoStack
        displayMode: FluNavigationViewType.Open

        logo: "qrc:/icons/logo.svg"

        title: "QChat"

        Component.onCompleted: {
            nav.setCurrentIndex(0)
        }

        items: FluObject {
            FluPaneItem {
                id: newChatItem
                title: "New Chat"
                icon: FluentIcons.Add
                onTapListener: function() {
                    newConversationDialog.open()
                }
            }

            FluPaneItemSeparator {}

            Repeater {
                visible: false
                model: chatManager.conversationModel
                delegate: FluPaneItem {
                    title: model.title || "New Chat"
                    icon: FluentIcons.ChatBubbles
                    onTapListener: function() {
                        chatManager.switchConversation(index)
                        nav.push("qrc:/qt/qml/QChat/src/qml/ChatPage.qml")
                    }
                }
            }

            FluPaneItemSeparator {}

            FluPaneItem {
                title: "Calls"
                icon: FluentIcons.Phone
                onTapListener: function() {
                    nav.push("qrc:/qt/qml/QChat/src/qml/CallsPage.qml")
                }
            }

            FluPaneItem {
                title: "Agents"
                icon: FluentIcons.ChatBubbles
                onTapListener: function() {
                    nav.push("qrc:/qt/qml/QChat/src/qml/AgentsPage.qml")
                }
            }

            FluPaneItem {
                title: "Skills"
                icon: FluentIcons.Library
                onTapListener: function() {
                    nav.push("qrc:/qt/qml/QChat/src/qml/SkillsPage.qml")
                }
            }

            FluPaneItem {
                title: "MCP"
                icon: FluentIcons.Document
                onTapListener: function() {
                    nav.push("qrc:/qt/qml/QChat/src/qml/McpPage.qml")
                }
            }

            FluPaneItem {
                title: "Image Gen"
                icon: FluentIcons.Photo
                onTapListener: function() {
                    nav.push("qrc:/qt/qml/QChat/src/qml/ImageGenPage.qml")
                }
            }
        }

        footerItems: FluObject {
            FluPaneItem {
                title: "Settings"
                icon: FluentIcons.Settings
                onTapListener: function() {
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
                source: (chatManager.messageModel.count > 0
                    || chatManager.currentConversationId.length > 0)
                    ? "qrc:/qt/qml/QChat/src/qml/ChatPage.qml"
                    : "qrc:/qt/qml/QChat/src/qml/WelcomePage.qml"
            }
        }
    }

    NewConversationDialog {
        id: newConversationDialog
        onConversationCreated: {
            nav.push("qrc:/qt/qml/QChat/src/qml/ChatPage.qml")
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
