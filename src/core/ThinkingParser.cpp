#include "ThinkingParser.h"
#include <QRegularExpression>

ThinkingParser::ThinkingParser(QObject *parent) : QObject(parent) {}

bool ThinkingParser::hasThinking(const QString &text) const {
    return text.contains("<think>") || text.contains("</think>");
}

QString ThinkingParser::extractThinking(const QString &text) const {
    QRegularExpression re("<think>(.*?)</think>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(text);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    // Handle unclosed think tag (streaming)
    int start = text.indexOf("<think>");
    if (start >= 0) {
        return text.mid(start + 7).trimmed();
    }
    return {};
}

QString ThinkingParser::extractContent(const QString &text) const {
    QRegularExpression re("<think>.*?</think>", QRegularExpression::DotMatchesEverythingOption);
    QString result = text;
    result.remove(re);
    // Also remove unclosed think tag
    int start = result.indexOf("<think>");
    if (start >= 0) {
        result = result.left(start);
    }
    return result.trimmed();
}
