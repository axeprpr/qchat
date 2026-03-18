#include "ChatManager.h"
#include "OpenAIProvider.h"
#include "ClaudeProvider.h"
#include "OllamaProvider.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , m_messageModel(new MessageModel(this))
    , m_conversationModel(new ConversationListModel(this))
    , m_settings(new SettingsManager(this))
    , m_documentParser(new DocumentParser(this))
    , m_markdown(new MarkdownHelper(this))
    , m_thinkingParser(new ThinkingParser(this))
{
    m_dataFilePath = m_settings->dataPath() + "/conversations.json";
    initProviders();
    loadConversations();

    connect(m_conversationModel, &ConversationListModel::conversationSelected,
            this, [this](const QString &id, const QJsonArray &messages) {
        m_currentConversationId = id;
        m_messageModel->fromJsonArray(messages);
        emit currentConversationIdChanged();
    });
}

void ChatManager::initProviders() {
    auto *openai = new OpenAIProvider(this);
    ProviderConfig openaiConfig;
    openaiConfig.name = "OpenAI";
    openaiConfig.defaultModel = "gpt-4o";
    QJsonObject cfg = m_settings->getProviderConfig("OpenAI");
    openaiConfig.apiKey = cfg["apiKey"].toString();
    openaiConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        openaiConfig.defaultModel = cfg["defaultModel"].toString();
    openai->setConfig(openaiConfig);
    m_providers["OpenAI"] = openai;

    auto *claude = new ClaudeProvider(this);
    ProviderConfig claudeConfig;
    claudeConfig.name = "Claude";
    claudeConfig.defaultModel = "claude-sonnet-4-6";
    cfg = m_settings->getProviderConfig("Claude");
    claudeConfig.apiKey = cfg["apiKey"].toString();
    claudeConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        claudeConfig.defaultModel = cfg["defaultModel"].toString();
    claude->setConfig(claudeConfig);
    m_providers["Claude"] = claude;

    auto *ollama = new OllamaProvider(this);
    ProviderConfig ollamaConfig;
    ollamaConfig.name = "Ollama";
    ollamaConfig.defaultModel = "llama3.3";
    cfg = m_settings->getProviderConfig("Ollama");
    ollamaConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        ollamaConfig.defaultModel = cfg["defaultModel"].toString();
    ollama->setConfig(ollamaConfig);
    m_providers["Ollama"] = ollama;

    // DeepSeek (OpenAI-compatible)
    auto *deepseek = new OpenAIProvider(this);
    ProviderConfig dsConfig;
    dsConfig.name = "DeepSeek";
    dsConfig.baseUrl = "https://api.deepseek.com/v1";
    dsConfig.defaultModel = "deepseek-chat";
    cfg = m_settings->getProviderConfig("DeepSeek");
    dsConfig.apiKey = cfg["apiKey"].toString();
    if (!cfg["baseUrl"].toString().isEmpty())
        dsConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        dsConfig.defaultModel = cfg["defaultModel"].toString();
    deepseek->setConfig(dsConfig);
    m_providers["DeepSeek"] = deepseek;

    // Google Gemini (OpenAI-compatible endpoint)
    auto *gemini = new OpenAIProvider(this);
    ProviderConfig geminiConfig;
    geminiConfig.name = "Gemini";
    geminiConfig.baseUrl = "https://generativelanguage.googleapis.com/v1beta/openai";
    geminiConfig.defaultModel = "gemini-2.5-pro";
    cfg = m_settings->getProviderConfig("Gemini");
    geminiConfig.apiKey = cfg["apiKey"].toString();
    if (!cfg["baseUrl"].toString().isEmpty())
        geminiConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        geminiConfig.defaultModel = cfg["defaultModel"].toString();
    gemini->setConfig(geminiConfig);
    m_providers["Gemini"] = gemini;
}

ModelProvider* ChatManager::currentProvider() {
    return m_providers.value(m_settings->currentProvider(), nullptr);
}

void ChatManager::sendMessage(const QString &content, const QStringList &attachments) {
    if (content.trimmed().isEmpty() && attachments.isEmpty()) return;
    if (m_isGenerating) return;

    // Create conversation if needed
    if (m_currentConversationId.isEmpty()) {
        m_currentConversationId = m_conversationModel->createConversation();
        emit currentConversationIdChanged();
    }

    // Build message with document context
    QString fullContent = content;
    if (!attachments.isEmpty()) {
        fullContent = buildDocumentContext(attachments) + "\n\n" + content;
    }

    m_messageModel->addMessage("user", content, attachments);

    auto *provider = currentProvider();
    if (!provider) {
        m_messageModel->addErrorMessage("No provider configured. Please set up a provider in Settings.");
        return;
    }

    if (provider->config().apiKey.isEmpty() && m_settings->currentProvider() != "Ollama") {
        m_messageModel->addErrorMessage("API key not configured for " + m_settings->currentProvider() +
                                         ". Please add your API key in Settings.");
        return;
    }

    // Prepare messages with system prompt
    QList<ChatMessage> chatMessages;
    if (!m_settings->systemPrompt().isEmpty()) {
        chatMessages.append({"system", m_settings->systemPrompt(), "", {}});
    }

    // Add conversation history (with document context for attached files)
    auto history = m_messageModel->toChatMessages();
    // Replace last user message content with full content (including document context)
    if (!history.isEmpty() && history.last().role == "user") {
        history.last().content = fullContent;
    }
    chatMessages.append(history);

    m_isGenerating = true;
    emit isGeneratingChanged();

    m_messageModel->startAssistantMessage();

    // Connect provider signals
    connect(provider, &ModelProvider::responseChunk, this, &ChatManager::onResponseChunk);
    connect(provider, &ModelProvider::thinkingChunk, this, &ChatManager::onThinkingChunk);
    connect(provider, &ModelProvider::responseFinished, this, &ChatManager::onResponseFinished);
    connect(provider, &ModelProvider::errorOccurred, this, &ChatManager::onProviderError);

    provider->sendMessage(chatMessages, m_settings->currentModel(),
                          m_settings->temperature(), m_settings->maxTokens(),
                          m_settings->enableThinking());
}

void ChatManager::onResponseChunk(const QString &text) {
    m_messageModel->appendToLastMessage(text);
}

void ChatManager::onThinkingChunk(const QString &text) {
    m_messageModel->appendThinking(text);
}

void ChatManager::onResponseFinished(const QString &response, const QString &thinking) {
    Q_UNUSED(response)
    Q_UNUSED(thinking)
    m_messageModel->finishStreaming();
    m_isGenerating = false;
    emit isGeneratingChanged();
    saveCurrentConversation();
    disconnectProvider();
}

void ChatManager::onProviderError(const QString &errorMsg) {
    m_messageModel->finishStreaming();
    if (m_messageModel->lastAssistantMessage().isEmpty()) {
        // Remove the empty assistant message and add error
        // For simplicity, just mark the last message as error
    }
    m_messageModel->addErrorMessage("Error: " + errorMsg);
    m_isGenerating = false;
    emit isGeneratingChanged();
    emit error(errorMsg);
    disconnectProvider();
}

void ChatManager::disconnectProvider() {
    auto *provider = currentProvider();
    if (provider) {
        disconnect(provider, &ModelProvider::responseChunk, this, &ChatManager::onResponseChunk);
        disconnect(provider, &ModelProvider::thinkingChunk, this, &ChatManager::onThinkingChunk);
        disconnect(provider, &ModelProvider::responseFinished, this, &ChatManager::onResponseFinished);
        disconnect(provider, &ModelProvider::errorOccurred, this, &ChatManager::onProviderError);
    }
}

void ChatManager::stopGeneration() {
    auto *provider = currentProvider();
    if (provider) {
        provider->cancelRequest();
    }
    m_messageModel->finishStreaming();
    m_isGenerating = false;
    emit isGeneratingChanged();
    saveCurrentConversation();
    disconnectProvider();
}

void ChatManager::newConversation() {
    saveCurrentConversation();
    m_currentConversationId = m_conversationModel->createConversation();
    m_messageModel->clear();
    emit currentConversationIdChanged();
}

void ChatManager::switchConversation(int index) {
    saveCurrentConversation();
    m_conversationModel->setCurrentIndex(index);
}

void ChatManager::deleteConversation(int index) {
    m_conversationModel->deleteConversation(index);
    if (m_conversationModel->rowCount() == 0) {
        m_currentConversationId.clear();
        m_messageModel->clear();
        emit currentConversationIdChanged();
    }
}

void ChatManager::clearCurrentConversation() {
    m_messageModel->clear();
    saveCurrentConversation();
}

void ChatManager::saveCurrentConversation() {
    if (m_currentConversationId.isEmpty()) return;
    m_conversationModel->updateConversation(
        m_currentConversationId,
        m_messageModel->toJsonArray(),
        m_messageModel->lastAssistantMessage(),
        m_settings->currentModel()
    );
    m_conversationModel->saveToFile(m_dataFilePath);
}

void ChatManager::loadConversations() {
    m_conversationModel->loadFromFile(m_dataFilePath);
}

void ChatManager::exportConversation(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    auto messages = m_messageModel->toChatMessages();
    for (const auto &msg : messages) {
        out << "## " << msg.role.toUpper() << "\n\n";
        if (!msg.thinkingContent.isEmpty())
            out << "> **Thinking:**\n> " << QString(msg.thinkingContent).replace("\n", "\n> ") << "\n\n";
        out << msg.content << "\n\n---\n\n";
    }
}

QString ChatManager::buildDocumentContext(const QStringList &attachments) {
    if (attachments.isEmpty()) return {};

    QString context = "The following documents are attached:\n\n";
    for (const auto &path : attachments) {
        QString desc = m_documentParser->fileDescription(path);
        QString content = m_documentParser->parseFile(path);
        context += "### " + desc + "\n" + content + "\n\n";
    }
    return context;
}

QStringList ChatManager::availableModels() const {
    auto *provider = m_providers.value(m_settings->currentProvider(), nullptr);
    if (provider) return provider->defaultModels();
    return {};
}

void ChatManager::fetchRemoteModels() {
    auto *provider = currentProvider();
    if (auto *openai = qobject_cast<OpenAIProvider*>(provider)) {
        openai->fetchModels();
    } else if (auto *ollama = qobject_cast<OllamaProvider*>(provider)) {
        ollama->fetchModels();
    }
}

void ChatManager::retryLastMessage() {
    if (m_isGenerating) return;

    auto messages = m_messageModel->toChatMessages();
    if (messages.isEmpty()) return;

    // Find the last user message
    QString lastUserMsg;
    QStringList lastAttachments;
    for (int i = messages.count() - 1; i >= 0; --i) {
        if (messages[i].role == "user") {
            lastUserMsg = messages[i].content;
            lastAttachments = messages[i].attachments;
            break;
        }
    }

    if (lastUserMsg.isEmpty()) return;

    // Remove the last assistant message(s)
    // For simplicity, clear and re-add all messages except the last assistant one
    // Then resend
    sendMessage(lastUserMsg, lastAttachments);
}

void ChatManager::setChatMode(const QString &mode) {
    if (m_chatMode != mode) {
        m_chatMode = mode;
        emit chatModeChanged();
    }
}

void ChatManager::setDeepResearch(bool val) {
    if (m_deepResearch != val) {
        m_deepResearch = val;
        emit deepResearchChanged();
    }
}

void ChatManager::addImageAttachment(const QString &path) {
    if (!m_attachments.contains(path)) {
        m_attachments.append(path);
        emit attachmentsChanged();
    }
}

void ChatManager::removeAttachment(int index) {
    if (index >= 0 && index < m_attachments.count()) {
        m_attachments.removeAt(index);
        emit attachmentsChanged();
    }
}

void ChatManager::clearAttachments() {
    if (!m_attachments.isEmpty()) {
        m_attachments.clear();
        emit attachmentsChanged();
    }
}
