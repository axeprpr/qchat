#pragma once

#include "ModelProvider.h"

class ExternalAgentProvider : public ModelProvider {
    Q_OBJECT
public:
    explicit ExternalAgentProvider(const QString &name, QObject *parent = nullptr);

    QString providerName() const override;
    void sendMessage(const QList<ChatMessage> &messages,
                     const QString &model,
                     double temperature,
                     int maxTokens,
                     bool enableThinking) override;
    void cancelRequest() override;
    QStringList defaultModels() const override;
    void setConversationContext(const QJsonObject &context) override;

private slots:
    void onStreamData();
    void onReplyFinished();

private:
    bool sendDifyMessage(const QString &query);
    bool sendDeerFlowMessage(const QString &query);
    QString ensureDeerFlowThreadId();
    void emitSessionIfReady(const QString &messageId = QString());
    QString normalizeBaseUrl() const;
    QString extractTextContent(const QJsonValue &contentVal) const;
    void parseSSELine(const QString &line);
    void handleDifyEvent(const QJsonObject &obj);
    void handleDeerFlowEvent(const QString &eventName, const QJsonValue &dataVal);

    QString m_name;
    QJsonObject m_context;
    QString m_fullResponse;
    QString m_thinkingContent;
    QByteArray m_buffer;
    QString m_sseEvent;
    QString m_threadId;
    QString m_runId;
};
