#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QFont>
#include <QDebug>

#include "core/ChatManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QChat");
    app.setOrganizationName("QChat");
    app.setApplicationVersion("1.0.0");

    // Set default style
    QQuickStyle::setStyle("Basic");

    // Set default font
    QFont font("Segoe UI", 10);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);

    // Create core manager
    ChatManager chatManager;

    QQmlApplicationEngine engine;

    // Expose to QML
    engine.rootContext()->setContextProperty("chatManager", &chatManager);

    // Load QML using module URI (Qt 6.5+ recommended way)
    engine.loadFromModule("QChat", "Main");

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML. Check that QML files are properly compiled.";
        return -1;
    }

    return app.exec();
}
