#include "DocumentParser.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>

DocumentParser::DocumentParser(QObject *parent) : QObject(parent) {}

QStringList DocumentParser::supportedFormats() const {
    return {
        "txt", "md", "markdown", "csv", "json", "xml", "html", "htm",
        "py", "js", "ts", "cpp", "c", "h", "hpp", "java", "rs", "go",
        "rb", "php", "swift", "kt", "cs", "sh", "bash", "yaml", "yml",
        "toml", "ini", "cfg", "conf", "log", "sql", "r", "m", "tex",
    };
}

bool DocumentParser::isSupported(const QString &filePath) const {
    QFileInfo info(filePath);
    return supportedFormats().contains(info.suffix().toLower());
}

QString DocumentParser::fileDescription(const QString &filePath) const {
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    qint64 size = info.size();

    QString sizeStr;
    if (size < 1024) sizeStr = QString::number(size) + " B";
    else if (size < 1048576) sizeStr = QString::number(size / 1024.0, 'f', 1) + " KB";
    else sizeStr = QString::number(size / 1048576.0, 'f', 1) + " MB";

    return info.fileName() + " (" + sizeStr + ")";
}

QString DocumentParser::parseFile(const QString &filePath) {
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();

    if (suffix == "csv") return parseCsvFile(filePath);
    if (suffix == "json") return parseJsonFile(filePath);
    if (suffix == "xml") return parseXmlFile(filePath);
    if (suffix == "html" || suffix == "htm") return parseHtmlFile(filePath);
    if (suffix == "md" || suffix == "markdown") return parseMarkdownFile(filePath);

    // Code files and plain text
    QStringList codeExts = {"py","js","ts","cpp","c","h","hpp","java","rs","go",
                            "rb","php","swift","kt","cs","sh","bash","sql","r","m"};
    if (codeExts.contains(suffix)) return parseCodeFile(filePath);

    return parseTextFile(filePath);
}

QString DocumentParser::parseTextFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return "Error: Cannot open file.";
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString content = stream.readAll();
    // Truncate very large files
    if (content.length() > 100000)
        content = content.left(100000) + "\n\n[... truncated, " + QString::number(content.length()) + " chars total]";
    return content;
}

QString DocumentParser::parseCsvFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return "Error: Cannot open file.";
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QString result = "CSV Data:\n";
    int lineCount = 0;
    while (!stream.atEnd() && lineCount < 500) {
        result += stream.readLine() + "\n";
        lineCount++;
    }
    if (!stream.atEnd())
        result += "\n[... truncated, showing first 500 rows]";
    return result;
}

QString DocumentParser::parseJsonFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return "Error: Cannot open file.";
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return "JSON Content:\n```json\n" + QString::fromUtf8(doc.toJson(QJsonDocument::Indented)) + "\n```";
}

QString DocumentParser::parseXmlFile(const QString &path) {
    return "XML File:\n```xml\n" + parseTextFile(path) + "\n```";
}

QString DocumentParser::parseHtmlFile(const QString &path) {
    // Simple HTML to text - strip tags
    QString html = parseTextFile(path);
    QString text;
    bool inTag = false;
    for (const QChar &c : html) {
        if (c == '<') { inTag = true; continue; }
        if (c == '>') { inTag = false; text += ' '; continue; }
        if (!inTag) text += c;
    }
    // Clean up multiple spaces/newlines
    return "HTML Content:\n" + text.simplified();
}

QString DocumentParser::parseMarkdownFile(const QString &path) {
    return parseTextFile(path);
}

QString DocumentParser::parseCodeFile(const QString &path) {
    QFileInfo info(path);
    return "Code (" + info.suffix() + "):\n```" + info.suffix() + "\n" + parseTextFile(path) + "\n```";
}
