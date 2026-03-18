#pragma once

#include "ModelProvider.h"

class ClaudeProvider : public ModelProvider {
    Q_OBJECT
public:
    explicit ClaudeProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void sendMessage(const QList<ChatMessage> &messages,
                     const QString &model,
                     double temperature,
                     int maxTokens,
                     bool enableThinking) override;
    void cancelRequest() override;
    QStringList defaultModels() const override;
    bool supportsThinking() const override { return true; }
    bool supportsVision() const override { return true; }

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
    void parseSSEEvent(const QString &event, const QString &data);

    QString m_fullResponse;
    QString m_thinkingContent;
    QByteArray m_buffer;
    QString m_currentEvent;
};
