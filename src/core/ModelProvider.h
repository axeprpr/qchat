#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct ChatMessage {
    QString role;      // "system", "user", "assistant"
    QString content;
    QString thinkingContent;  // For deep thinking models
    QStringList attachments;  // File paths
};

struct ProviderConfig {
    QString name;
    QString apiKey;
    QString baseUrl;
    QString defaultModel;
    bool supportsStreaming = true;
    bool supportsVision = false;
    bool supportsThinking = false;
    QStringList availableModels;
};

class ModelProvider : public QObject {
    Q_OBJECT
public:
    explicit ModelProvider(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ModelProvider() = default;

    virtual QString providerName() const = 0;
    virtual void sendMessage(const QList<ChatMessage> &messages,
                             const QString &model,
                             double temperature,
                             int maxTokens,
                             bool enableThinking) = 0;
    virtual void cancelRequest() = 0;
    virtual QStringList defaultModels() const = 0;
    virtual bool supportsThinking() const { return false; }
    virtual bool supportsVision() const { return false; }

    void setConfig(const ProviderConfig &config) { m_config = config; }
    ProviderConfig config() const { return m_config; }

signals:
    void responseChunk(const QString &text);
    void thinkingChunk(const QString &text);
    void responseFinished(const QString &fullResponse, const QString &thinkingContent);
    void errorOccurred(const QString &error);
    void modelsLoaded(const QStringList &models);

protected:
    ProviderConfig m_config;
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;
};
