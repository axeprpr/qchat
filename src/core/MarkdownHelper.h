#pragma once

#include <QObject>
#include <QString>

class MarkdownHelper : public QObject {
    Q_OBJECT
public:
    explicit MarkdownHelper(QObject *parent = nullptr);

    // Convert markdown to styled HTML for display in QML Text/TextArea
    Q_INVOKABLE QString toHtml(const QString &markdown) const;
    Q_INVOKABLE QString escapeHtml(const QString &text) const;

private:
    QString processInlineMarkdown(const QString &line) const;
    QString highlightCode(const QString &code, const QString &lang) const;
};
