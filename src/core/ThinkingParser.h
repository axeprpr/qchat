#pragma once

#include <QObject>
#include <QString>

// Parses <think>...</think> blocks from AI responses
class ThinkingParser : public QObject {
    Q_OBJECT
public:
    explicit ThinkingParser(QObject *parent = nullptr);

    Q_INVOKABLE bool hasThinking(const QString &text) const;
    Q_INVOKABLE QString extractThinking(const QString &text) const;
    Q_INVOKABLE QString extractContent(const QString &text) const;
};
