#pragma once

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include "MessageModel.h"
#include "ConversationListModel.h"
#include "ModelProvider.h"
#include "SettingsManager.h"
#include "DocumentParser.h"
#include "MarkdownHelper.h"
#include "ThinkingParser.h"
#include "ImageHelper.h"
#include "ExportHelper.h"
#include "PromptLibrary.h"
#include "ImageGenProvider.h"

class ChatManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(MessageModel* messageModel READ messageModel CONSTANT)
    Q_PROPERTY(ConversationListModel* conversationModel READ conversationModel CONSTANT)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(DocumentParser* documentParser READ documentParser CONSTANT)
    Q_PROPERTY(MarkdownHelper* markdown READ markdown CONSTANT)
    Q_PROPERTY(ThinkingParser* thinkingParser READ thinkingParser CONSTANT)
    Q_PROPERTY(ImageHelper* imageHelper READ imageHelper CONSTANT)
    Q_PROPERTY(ExportHelper* exportHelper READ exportHelper CONSTANT)
    Q_PROPERTY(PromptLibrary* promptLibrary READ promptLibrary CONSTANT)
    Q_PROPERTY(ImageGenProvider* imageGen READ imageGen CONSTANT)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(QString currentConversationId READ currentConversationId NOTIFY currentConversationIdChanged)
    Q_PROPERTY(QString chatMode READ chatMode WRITE setChatMode NOTIFY chatModeChanged)
    Q_PROPERTY(bool deepResearch READ deepResearch WRITE setDeepResearch NOTIFY deepResearchChanged)
    Q_PROPERTY(QStringList attachments READ attachments NOTIFY attachmentsChanged)

public:
    explicit ChatManager(QObject *parent = nullptr);

    MessageModel* messageModel() const { return m_messageModel; }
    ConversationListModel* conversationModel() const { return m_conversationModel; }
    SettingsManager* settings() const { return m_settings; }
    DocumentParser* documentParser() const { return m_documentParser; }
    MarkdownHelper* markdown() const { return m_markdown; }
    ThinkingParser* thinkingParser() const { return m_thinkingParser; }
    ImageHelper* imageHelper() const { return m_imageHelper; }
    ExportHelper* exportHelper() const { return m_exportHelper; }
    PromptLibrary* promptLibrary() const { return m_promptLibrary; }
    ImageGenProvider* imageGen() const { return m_imageGen; }
    bool isGenerating() const { return m_isGenerating; }
    QString currentConversationId() const { return m_currentConversationId; }
    QString chatMode() const { return m_chatMode; }
    void setChatMode(const QString &mode);
    bool deepResearch() const { return m_deepResearch; }
    void setDeepResearch(bool val);
    QStringList attachments() const { return m_attachments; }

    Q_INVOKABLE void sendMessage(const QString &content, const QStringList &attachments = {});
    Q_INVOKABLE void stopGeneration();
    Q_INVOKABLE void newConversation();
    Q_INVOKABLE void newConversationWithOptions(const QString &title, const QString &provider, bool newAgentSession = true);
    Q_INVOKABLE void switchConversation(int index);
    Q_INVOKABLE void deleteConversation(int index);
    Q_INVOKABLE void clearCurrentConversation();
    Q_INVOKABLE void exportConversation(const QString &path);
    Q_INVOKABLE QStringList availableModels() const;
    Q_INVOKABLE void fetchRemoteModels();
    Q_INVOKABLE void retryLastMessage();
    Q_INVOKABLE void addImageAttachment(const QString &path);
    Q_INVOKABLE void removeAttachment(int index);
    Q_INVOKABLE void clearAttachments();
    Q_INVOKABLE QStringList providerNames() const;
    Q_INVOKABLE bool isExternalProvider(const QString &provider) const;
    Q_INVOKABLE QJsonArray agents() const;
    Q_INVOKABLE QJsonObject agentById(const QString &id) const;
    Q_INVOKABLE void saveAgent(const QJsonObject &agent);
    Q_INVOKABLE void deleteAgent(const QString &id);
    Q_INVOKABLE QString conversationAgentId() const;
    Q_INVOKABLE QString conversationAgentName() const;
    Q_INVOKABLE void setConversationAgentId(const QString &agentId);
    Q_INVOKABLE QJsonArray skills() const;
    Q_INVOKABLE QJsonObject skillById(const QString &id) const;
    Q_INVOKABLE void saveSkill(const QJsonObject &skill);
    Q_INVOKABLE void deleteSkill(const QString &id);
    Q_INVOKABLE QJsonArray mcpServers() const;
    Q_INVOKABLE QJsonObject mcpServerById(const QString &id) const;
    Q_INVOKABLE void saveMcpServer(const QJsonObject &server);
    Q_INVOKABLE void deleteMcpServer(const QString &id);
    Q_INVOKABLE QJsonArray conversationSkillIds() const;
    Q_INVOKABLE QJsonArray conversationMcpServerIds() const;
    Q_INVOKABLE void setConversationSkillIds(const QVariantList &ids);
    Q_INVOKABLE void setConversationMcpServerIds(const QVariantList &ids);
    Q_INVOKABLE QStringList conversationSkillNames() const;
    Q_INVOKABLE QStringList conversationMcpServerNames() const;

signals:
    void isGeneratingChanged();
    void currentConversationIdChanged();
    void chatModeChanged();
    void deepResearchChanged();
    void attachmentsChanged();
    void agentsChanged();
    void conversationAgentChanged();
    void skillsChanged();
    void mcpServersChanged();
    void conversationCapabilitiesChanged();
    void error(const QString &message);

private slots:
    void onResponseChunk(const QString &text);
    void onThinkingChunk(const QString &text);
    void onResponseFinished(const QString &response, const QString &thinking);
    void onProviderError(const QString &errorMsg);
    void onProviderSessionUpdated(const QJsonObject &session);

private:
    void initProviders();
    ModelProvider* currentProvider();
    QString currentProviderName() const;
    void disconnectProvider();
    void saveCurrentConversation();
    void loadConversations();
    QString buildDocumentContext(const QStringList &attachments);
    QJsonArray normalizeIdArray(const QJsonArray &ids) const;
    QJsonArray effectiveSkillIds() const;
    QJsonArray effectiveMcpServerIds() const;
    QJsonArray resolveSkills(const QJsonArray &ids) const;
    QJsonArray resolveMcpServers(const QJsonArray &ids) const;
    QString buildCapabilityInstruction(const QJsonArray &skills, const QJsonArray &mcpServers) const;

    MessageModel *m_messageModel;
    ConversationListModel *m_conversationModel;
    SettingsManager *m_settings;
    DocumentParser *m_documentParser;
    MarkdownHelper *m_markdown;
    ThinkingParser *m_thinkingParser;
    ImageHelper *m_imageHelper;
    ExportHelper *m_exportHelper;
    PromptLibrary *m_promptLibrary;
    ImageGenProvider *m_imageGen;

    QMap<QString, ModelProvider*> m_providers;
    ModelProvider *m_activeProvider = nullptr;
    bool m_isGenerating = false;
    QString m_currentConversationId;
    QString m_dataFilePath;
    QString m_chatMode = "quick";
    bool m_deepResearch = false;
    QStringList m_attachments;
    QString m_pendingAgentId;
    QJsonArray m_pendingSkillIds;
    QJsonArray m_pendingMcpServerIds;
};
