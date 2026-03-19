#pragma once

#include <QObject>
#include <QString>
#include <QJsonArray>

class ExportHelper : public QObject {
    Q_OBJECT
public:
    explicit ExportHelper(QObject *parent = nullptr);

    enum ExportFormat {
        Markdown,
        HTML,
        PDF
    };
    Q_ENUM(ExportFormat)

    Q_INVOKABLE bool exportConversation(const QJsonArray &messages,
                                        const QString &filePath,
                                        ExportFormat format);

private:
    QString toMarkdown(const QJsonArray &messages);
    QString toHTML(const QJsonArray &messages);
    bool toPDF(const QJsonArray &messages, const QString &filePath);

    QString escapeHtml(const QString &text);
    QString getHTMLTemplate();
};
