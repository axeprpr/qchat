#pragma once

#include "ModelProvider.h"

// Supports OpenAI, DeepSeek, Groq, Together, OpenRouter, and any OpenAI-compatible API
class OpenAIProvider : public ModelProvider {
    Q_OBJECT
public:
    explicit OpenAIProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void sendMessage(const QList<ChatMessage> &messages,
                     const QString &model,
                     double temperature,
                     int maxTokens,
                     bool enableThinking) override;
    void cancelRequest() override;
    QStringList defaultModels() const override;
    bool supportsThinking() const override;
    bool supportsVision() const override;

    void fetchModels();

private slots:
    void onStreamData();
    void onReplyFinished();

private:
    QJsonObject buildRequestBody(const QList<ChatMessage> &messages,
                                 const QString &model,
                                 double temperature,
                                 int maxTokens,
                                 bool enableThinking);
    QJsonArray messagesToJson(const QList<ChatMessage> &messages);
    void parseSSELine(const QString &line);

    QString m_fullResponse;
    QString m_thinkingContent;
    bool m_inThinking = false;
    QByteArray m_buffer;
};
