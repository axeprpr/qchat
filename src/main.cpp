#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QFont>

#include "core/ChatManager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("QChat");
    app.setOrganizationName("QChat");
    app.setApplicationVersion("1.0.0");

    // Set default style
    QQuickStyle::setStyle("Basic");

    // Set default font
    QFont font("Segoe UI", 10);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);

    // Register types
    qmlRegisterUncreatableType<MessageModel>("QChat.Core", 1, 0, "MessageModel", "Access via chatManager");
    qmlRegisterUncreatableType<ConversationListModel>("QChat.Core", 1, 0, "ConversationListModel", "Access via chatManager");
    qmlRegisterUncreatableType<SettingsManager>("QChat.Core", 1, 0, "SettingsManager", "Access via chatManager");

    // Create core manager
    ChatManager chatManager;

    QQmlApplicationEngine engine;

    // Expose to QML
    engine.rootContext()->setContextProperty("chatManager", &chatManager);

    const QUrl url(u"qrc:/QChat/src/qml/Main.qml"_qs);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
