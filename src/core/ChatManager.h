#pragma once

#include <QObject>
#include <QMap>
#include "MessageModel.h"
#include "ConversationListModel.h"
#include "ModelProvider.h"
#include "SettingsManager.h"
#include "DocumentParser.h"
#include "MarkdownHelper.h"
#include "ThinkingParser.h"

class ChatManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(MessageModel* messageModel READ messageModel CONSTANT)
    Q_PROPERTY(ConversationListModel* conversationModel READ conversationModel CONSTANT)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(DocumentParser* documentParser READ documentParser CONSTANT)
    Q_PROPERTY(MarkdownHelper* markdown READ markdown CONSTANT)
    Q_PROPERTY(ThinkingParser* thinkingParser READ thinkingParser CONSTANT)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(QString currentConversationId READ currentConversationId NOTIFY currentConversationIdChanged)

public:
    explicit ChatManager(QObject *parent = nullptr);

    MessageModel* messageModel() const { return m_messageModel; }
    ConversationListModel* conversationModel() const { return m_conversationModel; }
    SettingsManager* settings() const { return m_settings; }
    DocumentParser* documentParser() const { return m_documentParser; }
    MarkdownHelper* markdown() const { return m_markdown; }
    ThinkingParser* thinkingParser() const { return m_thinkingParser; }
    bool isGenerating() const { return m_isGenerating; }
    QString currentConversationId() const { return m_currentConversationId; }

    Q_INVOKABLE void sendMessage(const QString &content, const QStringList &attachments = {});
    Q_INVOKABLE void stopGeneration();
    Q_INVOKABLE void newConversation();
    Q_INVOKABLE void switchConversation(int index);
    Q_INVOKABLE void deleteConversation(int index);
    Q_INVOKABLE void clearCurrentConversation();
    Q_INVOKABLE void exportConversation(const QString &path);
    Q_INVOKABLE QStringList availableModels() const;
    Q_INVOKABLE void fetchRemoteModels();
    Q_INVOKABLE void retryLastMessage();

signals:
    void isGeneratingChanged();
    void currentConversationIdChanged();
    void error(const QString &message);

private slots:
    void onResponseChunk(const QString &text);
    void onThinkingChunk(const QString &text);
    void onResponseFinished(const QString &response, const QString &thinking);
    void onProviderError(const QString &errorMsg);

private:
    void initProviders();
    ModelProvider* currentProvider();
    void disconnectProvider();
    void saveCurrentConversation();
    void loadConversations();
    QString buildDocumentContext(const QStringList &attachments);

    MessageModel *m_messageModel;
    ConversationListModel *m_conversationModel;
    SettingsManager *m_settings;
    DocumentParser *m_documentParser;
    MarkdownHelper *m_markdown;
    ThinkingParser *m_thinkingParser;

    QMap<QString, ModelProvider*> m_providers;
    bool m_isGenerating = false;
    QString m_currentConversationId;
    QString m_dataFilePath;
};
