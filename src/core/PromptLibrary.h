#pragma once

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class PromptLibrary : public QObject {
    Q_OBJECT
    Q_PROPERTY(QJsonArray prompts READ prompts NOTIFY promptsChanged)

public:
    explicit PromptLibrary(QObject *parent = nullptr);

    QJsonArray prompts() const { return m_prompts; }

    Q_INVOKABLE void addPrompt(const QString &title, const QString &content, const QString &category = "General");
    Q_INVOKABLE void updatePrompt(int index, const QString &title, const QString &content, const QString &category);
    Q_INVOKABLE void deletePrompt(int index);
    Q_INVOKABLE QString getContent(int index) const;
    Q_INVOKABLE QJsonArray search(const QString &query) const;

signals:
    void promptsChanged();

private:
    void load();
    void save();

    QJsonArray m_prompts;
    QString m_filePath;
};
