#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class DocumentParser : public QObject {
    Q_OBJECT
public:
    explicit DocumentParser(QObject *parent = nullptr);

    Q_INVOKABLE QString parseFile(const QString &filePath);
    Q_INVOKABLE QStringList supportedFormats() const;
    Q_INVOKABLE bool isSupported(const QString &filePath) const;
    Q_INVOKABLE QString fileDescription(const QString &filePath) const;

private:
    QString parseTextFile(const QString &path);
    QString parseCsvFile(const QString &path);
    QString parseJsonFile(const QString &path);
    QString parseXmlFile(const QString &path);
    QString parseHtmlFile(const QString &path);
    QString parseMarkdownFile(const QString &path);
    QString parseCodeFile(const QString &path);
};
