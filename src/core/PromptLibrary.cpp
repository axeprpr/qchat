#include "PromptLibrary.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>

PromptLibrary::PromptLibrary(QObject *parent) : QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = dataDir + "/prompts.json";
    load();

    // Seed with built-in prompts if empty
    if (m_prompts.isEmpty()) {
        addPrompt("Explain Code", "Please explain the following code in detail:\n\n", "Development");
        addPrompt("Code Review", "Please review this code for bugs, performance issues, and best practices:\n\n", "Development");
        addPrompt("Write Tests", "Write comprehensive unit tests for the following code:\n\n", "Development");
        addPrompt("Summarize", "Please provide a concise summary of the following text:\n\n", "Writing");
        addPrompt("Translate to English", "Please translate the following text to English:\n\n", "Writing");
        addPrompt("Fix Grammar", "Please fix any grammar and spelling errors in the following text:\n\n", "Writing");
        addPrompt("Brainstorm Ideas", "Please brainstorm 10 creative ideas for:\n\n", "Creativity");
        addPrompt("Debug Help", "I'm getting the following error. Please help me debug it:\n\n", "Development");
    }
}

void PromptLibrary::addPrompt(const QString &title, const QString &content, const QString &category) {
    QJsonObject obj;
    obj["title"] = title;
    obj["content"] = content;
    obj["category"] = category;
    m_prompts.append(obj);
    save();
    emit promptsChanged();
}

void PromptLibrary::updatePrompt(int index, const QString &title, const QString &content, const QString &category) {
    if (index < 0 || index >= m_prompts.size()) return;
    QJsonObject obj;
    obj["title"] = title;
    obj["content"] = content;
    obj["category"] = category;
    m_prompts[index] = obj;
    save();
    emit promptsChanged();
}

void PromptLibrary::deletePrompt(int index) {
    if (index < 0 || index >= m_prompts.size()) return;
    m_prompts.removeAt(index);
    save();
    emit promptsChanged();
}

QString PromptLibrary::getContent(int index) const {
    if (index < 0 || index >= m_prompts.size()) return {};
    return m_prompts[index].toObject()["content"].toString();
}

QJsonArray PromptLibrary::search(const QString &query) const {
    if (query.isEmpty()) return m_prompts;
    QJsonArray results;
    QString q = query.toLower();
    for (const auto &val : m_prompts) {
        QJsonObject obj = val.toObject();
        if (obj["title"].toString().toLower().contains(q) ||
            obj["content"].toString().toLower().contains(q) ||
            obj["category"].toString().toLower().contains(q)) {
            results.append(obj);
        }
    }
    return results;
}

void PromptLibrary::load() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) m_prompts = doc.array();
}

void PromptLibrary::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(m_prompts).toJson());
}
