#pragma once

#include "ModelProvider.h"

class OllamaProvider : public ModelProvider {
    Q_OBJECT
public:
    explicit OllamaProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void sendMessage(const QList<ChatMessage> &messages,
                     const QString &model,
                     double temperature,
                     int maxTokens,
                     bool enableThinking) override;
    void cancelRequest() override;
    QStringList defaultModels() const override;
    bool supportsThinking() const override { return true; }

    void fetchModels();

private slots:
    void onStreamData();
    void onReplyFinished();

private:
    QString m_fullResponse;
    QString m_thinkingContent;
    bool m_inThinking = false;
    QByteArray m_buffer;
};
