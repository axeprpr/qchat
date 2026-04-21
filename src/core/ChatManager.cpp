#include "ChatManager.h"
#include "OpenAIProvider.h"
#include "ClaudeProvider.h"
#include "OllamaProvider.h"
#include "ExternalAgentProvider.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QSet>

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , m_messageModel(new MessageModel(this))
    , m_conversationModel(new ConversationListModel(this))
    , m_settings(new SettingsManager(this))
    , m_documentParser(new DocumentParser(this))
    , m_markdown(new MarkdownHelper(this))
    , m_thinkingParser(new ThinkingParser(this))
    , m_imageHelper(new ImageHelper(this))
    , m_exportHelper(new ExportHelper(this))
    , m_promptLibrary(new PromptLibrary(this))
    , m_imageGen(new ImageGenProvider(this))
{
    // Initialize imageGen with OpenAI API key
    QJsonObject openaiCfg = m_settings->getProviderConfig("OpenAI");
    m_imageGen->setApiKey(openaiCfg["apiKey"].toString());
    if (!openaiCfg["baseUrl"].toString().isEmpty())
        m_imageGen->setBaseUrl(openaiCfg["baseUrl"].toString());
    m_dataFilePath = m_settings->dataPath() + "/conversations.json";
    initProviders();
    loadConversations();

    connect(m_conversationModel, &ConversationListModel::conversationSelected,
            this, [this](const QString &id, const QJsonArray &messages) {
        m_currentConversationId = id;
        m_messageModel->fromJsonArray(messages);
        emit currentConversationIdChanged();
        emit conversationAgentChanged();
        emit conversationCapabilitiesChanged();
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
    openaiConfig.extra = m_settings->getProviderExtra("OpenAI");
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
    claudeConfig.extra = m_settings->getProviderExtra("Claude");
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
    ollamaConfig.extra = m_settings->getProviderExtra("Ollama");
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
    dsConfig.extra = m_settings->getProviderExtra("DeepSeek");
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
    geminiConfig.extra = m_settings->getProviderExtra("Gemini");
    if (!cfg["baseUrl"].toString().isEmpty())
        geminiConfig.baseUrl = cfg["baseUrl"].toString();
    if (!cfg["defaultModel"].toString().isEmpty())
        geminiConfig.defaultModel = cfg["defaultModel"].toString();
    gemini->setConfig(geminiConfig);
    m_providers["Gemini"] = gemini;

    auto *dify = new ExternalAgentProvider("Dify", this);
    ProviderConfig difyConfig;
    difyConfig.name = "Dify";
    cfg = m_settings->getProviderConfig("Dify");
    difyConfig.apiKey = cfg["apiKey"].toString();
    difyConfig.baseUrl = cfg["baseUrl"].toString();
    difyConfig.defaultModel = cfg["defaultModel"].toString();
    difyConfig.extra = m_settings->getProviderExtra("Dify");
    dify->setConfig(difyConfig);
    m_providers["Dify"] = dify;

    auto *deerFlow = new ExternalAgentProvider("DeerFlow", this);
    ProviderConfig deerFlowConfig;
    deerFlowConfig.name = "DeerFlow";
    cfg = m_settings->getProviderConfig("DeerFlow");
    deerFlowConfig.apiKey = cfg["apiKey"].toString();
    deerFlowConfig.baseUrl = cfg["baseUrl"].toString();
    deerFlowConfig.defaultModel = cfg["defaultModel"].toString();
    deerFlowConfig.extra = m_settings->getProviderExtra("DeerFlow");
    deerFlow->setConfig(deerFlowConfig);
    m_providers["DeerFlow"] = deerFlow;
}

ModelProvider* ChatManager::currentProvider() {
    return m_providers.value(currentProviderName(), nullptr);
}

QString ChatManager::currentProviderName() const {
    QString providerName = m_settings->currentProvider();

    QString agentId = m_pendingAgentId.trimmed();
    if (!m_currentConversationId.isEmpty()) {
        agentId = m_conversationModel->getConversationAgentById(m_currentConversationId).trimmed();
        const QString conversationProvider = m_conversationModel->getConversationProviderById(m_currentConversationId).trimmed();
        if (!conversationProvider.isEmpty()) {
            providerName = conversationProvider;
        }
    }

    if (!agentId.isEmpty()) {
        const QJsonObject agent = m_settings->getAgentById(agentId);
        const QString agentProvider = agent.value("provider").toString().trimmed();
        if (!agentProvider.isEmpty()) {
            providerName = agentProvider;
        }
    }

    return providerName;
}

void ChatManager::sendMessage(const QString &content, const QStringList &attachments) {
    if (content.trimmed().isEmpty() && attachments.isEmpty()) return;
    if (m_isGenerating) return;

    // Create conversation if needed
    if (m_currentConversationId.isEmpty()) {
        m_currentConversationId = m_conversationModel->createConversation();
        const int index = m_conversationModel->currentIndex();
        if (index >= 0) {
            QJsonObject settings;
            settings["provider"] = currentProviderName();
            if (!m_pendingAgentId.trimmed().isEmpty()) {
                settings["agentId"] = m_pendingAgentId.trimmed();
            }
            settings["skillIds"] = m_pendingSkillIds;
            settings["mcpServerIds"] = m_pendingMcpServerIds;
            m_conversationModel->updateConversationSettings(index, settings);
            if (isExternalProvider(currentProviderName())) {
                m_conversationModel->updateConversationRuntime(index, QJsonObject{
                    {"conversation_id", ""},
                    {"last_message_id", ""},
                    {"run_id", ""},
                });
            }
        }
        emit currentConversationIdChanged();
    }

    // Build message with document context
    QString fullContent = content;
    if (!attachments.isEmpty()) {
        fullContent = buildDocumentContext(attachments) + "\n\n" + content;
    }

    m_messageModel->addMessage("user", content, attachments);

    auto *provider = currentProvider();
    const QString providerName = currentProviderName();
    if (!provider) {
        m_messageModel->addErrorMessage("No provider configured. Please set up a provider in Settings.");
        return;
    }

    const QJsonArray skillIds = effectiveSkillIds();
    const QJsonArray mcpServerIds = effectiveMcpServerIds();
    const QJsonArray selectedSkills = resolveSkills(skillIds);
    const QJsonArray selectedMcpServers = resolveMcpServers(mcpServerIds);

    // Refresh provider config from settings so changes take effect immediately.
    {
        const QString activeAgentId = conversationAgentId();
        const QJsonObject agent = activeAgentId.isEmpty() ? QJsonObject{} : m_settings->getAgentById(activeAgentId);

        ProviderConfig config = provider->config();
        const QJsonObject cfg = m_settings->getProviderConfig(providerName);
        config.apiKey = cfg["apiKey"].toString();
        if (!cfg["baseUrl"].toString().isEmpty() ||
            providerName == "OpenAI" || providerName == "Claude" || providerName == "Ollama" ||
            isExternalProvider(providerName)) {
            config.baseUrl = cfg["baseUrl"].toString();
        }
        if (!cfg["defaultModel"].toString().isEmpty()) {
            config.defaultModel = cfg["defaultModel"].toString();
        }
        config.extra = m_settings->getProviderExtra(providerName);

        if (!agent.isEmpty()) {
            if (!agent.value("apiKey").toString().trimmed().isEmpty()) {
                config.apiKey = agent.value("apiKey").toString().trimmed();
            }
            if (!agent.value("baseUrl").toString().trimmed().isEmpty()) {
                config.baseUrl = agent.value("baseUrl").toString().trimmed();
            }
            if (!agent.value("model").toString().trimmed().isEmpty()) {
                config.defaultModel = agent.value("model").toString().trimmed();
            }

            if (providerName == "Dify") {
                if (!agent.value("userId").toString().trimmed().isEmpty()) {
                    config.extra["userId"] = agent.value("userId").toString().trimmed();
                }
            } else if (providerName == "DeerFlow") {
                if (!agent.value("assistantId").toString().trimmed().isEmpty()) {
                    config.extra["assistantId"] = agent.value("assistantId").toString().trimmed();
                }
                if (!agent.value("mode").toString().trimmed().isEmpty()) {
                    config.extra["mode"] = agent.value("mode").toString().trimmed();
                }
                if (!agent.value("modelName").toString().trimmed().isEmpty()) {
                    config.extra["modelName"] = agent.value("modelName").toString().trimmed();
                }
                if (!agent.value("agentName").toString().trimmed().isEmpty()) {
                    config.extra["agentName"] = agent.value("agentName").toString().trimmed();
                }
                if (!agent.value("authToken").toString().trimmed().isEmpty()) {
                    config.extra["authToken"] = agent.value("authToken").toString().trimmed();
                }
            }
        }

        config.extra["qchat_skill_ids"] = skillIds;
        config.extra["qchat_mcp_server_ids"] = mcpServerIds;
        config.extra["qchat_skills"] = selectedSkills;
        config.extra["qchat_mcp_servers"] = selectedMcpServers;

        provider->setConfig(config);
    }

    if (provider->config().apiKey.isEmpty() && providerName != "Ollama" && !isExternalProvider(providerName)) {
        m_messageModel->addErrorMessage("API key not configured for " + providerName +
                                         ". Please add your API key in Settings.");
        return;
    }

    // Prepare messages with system prompt
    QList<ChatMessage> chatMessages;
    const int conversationIndex = m_conversationModel->currentIndex();
    const QJsonObject convSettings = m_conversationModel->getConversationSettings(conversationIndex);
    QString mergedSystemPrompt = m_settings->systemPrompt();
    const QString conversationSystemPrompt = convSettings.value("systemPrompt").toString().trimmed();
    if (!conversationSystemPrompt.isEmpty()) {
        mergedSystemPrompt = conversationSystemPrompt;
    }
    const QString capabilityInstruction = buildCapabilityInstruction(selectedSkills, selectedMcpServers);
    if (!capabilityInstruction.isEmpty()) {
        if (!mergedSystemPrompt.isEmpty()) {
            mergedSystemPrompt += "\n\n";
        }
        mergedSystemPrompt += capabilityInstruction;
    }
    if (!mergedSystemPrompt.isEmpty()) {
        chatMessages.append({"system", mergedSystemPrompt, "", {}});
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
    m_activeProvider = provider;
    QJsonObject providerContext = m_conversationModel->getConversationRuntimeById(m_currentConversationId);
    providerContext["qchat_skill_ids"] = skillIds;
    providerContext["qchat_mcp_server_ids"] = mcpServerIds;
    providerContext["qchat_skills"] = selectedSkills;
    providerContext["qchat_mcp_servers"] = selectedMcpServers;
    provider->setConversationContext(providerContext);
    connect(provider, &ModelProvider::responseChunk, this, &ChatManager::onResponseChunk);
    connect(provider, &ModelProvider::thinkingChunk, this, &ChatManager::onThinkingChunk);
    connect(provider, &ModelProvider::responseFinished, this, &ChatManager::onResponseFinished);
    connect(provider, &ModelProvider::errorOccurred, this, &ChatManager::onProviderError);
    connect(provider, &ModelProvider::sessionUpdated, this, &ChatManager::onProviderSessionUpdated);

    double requestTemperature = m_settings->temperature();
    if (convSettings.contains("temperature")) {
        const double conversationTemp = convSettings.value("temperature").toDouble(-1.0);
        if (conversationTemp >= 0.0) {
            requestTemperature = conversationTemp;
        }
    }

    provider->sendMessage(chatMessages, m_settings->currentModel(),
                          requestTemperature, m_settings->maxTokens(),
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

void ChatManager::onProviderSessionUpdated(const QJsonObject &session) {
    if (m_currentConversationId.isEmpty()) {
        return;
    }
    m_conversationModel->updateConversationRuntimeById(m_currentConversationId, session);
    m_conversationModel->saveToFile(m_dataFilePath);
}

void ChatManager::disconnectProvider() {
    auto *provider = m_activeProvider ? m_activeProvider : currentProvider();
    if (provider) {
        disconnect(provider, &ModelProvider::responseChunk, this, &ChatManager::onResponseChunk);
        disconnect(provider, &ModelProvider::thinkingChunk, this, &ChatManager::onThinkingChunk);
        disconnect(provider, &ModelProvider::responseFinished, this, &ChatManager::onResponseFinished);
        disconnect(provider, &ModelProvider::errorOccurred, this, &ChatManager::onProviderError);
        disconnect(provider, &ModelProvider::sessionUpdated, this, &ChatManager::onProviderSessionUpdated);
    }
    m_activeProvider = nullptr;
}

void ChatManager::stopGeneration() {
    auto *provider = m_activeProvider ? m_activeProvider : currentProvider();
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
    newConversationWithOptions("New Chat", m_settings->currentProvider(), true);
}

void ChatManager::newConversationWithOptions(const QString &title, const QString &provider, bool newAgentSession) {
    const QString targetProvider = provider.trimmed();
    QJsonObject inheritedRuntime;
    if (!newAgentSession && isExternalProvider(targetProvider) && !m_currentConversationId.isEmpty()) {
        const QString currentConversationProvider =
            m_conversationModel->getConversationProviderById(m_currentConversationId).trimmed();
        if (currentConversationProvider == targetProvider) {
            inheritedRuntime = m_conversationModel->getConversationRuntimeById(m_currentConversationId);
        }
    }

    saveCurrentConversation();
    m_currentConversationId = m_conversationModel->createConversation(title.trimmed().isEmpty() ? "New Chat" : title.trimmed());
    m_messageModel->clear();
    m_pendingAgentId.clear();
    m_pendingSkillIds = QJsonArray{};
    m_pendingMcpServerIds = QJsonArray{};

    const int index = m_conversationModel->currentIndex();
    if (index >= 0) {
        if (!targetProvider.isEmpty()) {
            QJsonObject settings;
            settings["provider"] = targetProvider;
            settings["agentId"] = "";
            settings["skillIds"] = QJsonArray{};
            settings["mcpServerIds"] = QJsonArray{};
            m_conversationModel->updateConversationSettings(index, settings);
        }
        if (isExternalProvider(targetProvider)) {
            if (newAgentSession || inheritedRuntime.isEmpty()) {
                m_conversationModel->updateConversationRuntime(index, QJsonObject{
                    {"conversation_id", ""},
                    {"last_message_id", ""},
                    {"run_id", ""},
                });
            } else {
                m_conversationModel->updateConversationRuntime(index, inheritedRuntime);
            }
        }
    }
    m_conversationModel->saveToFile(m_dataFilePath);
    emit currentConversationIdChanged();
    emit conversationAgentChanged();
    emit conversationCapabilitiesChanged();
}

void ChatManager::switchConversation(int index) {
    saveCurrentConversation();
    m_conversationModel->setCurrentIndex(index);
    m_pendingAgentId.clear();
    m_pendingSkillIds = QJsonArray{};
    m_pendingMcpServerIds = QJsonArray{};
    emit conversationAgentChanged();
    emit conversationCapabilitiesChanged();
}

void ChatManager::deleteConversation(int index) {
    m_conversationModel->deleteConversation(index);
    if (m_conversationModel->rowCount() == 0) {
        m_currentConversationId.clear();
        m_pendingAgentId.clear();
        m_pendingSkillIds = QJsonArray{};
        m_pendingMcpServerIds = QJsonArray{};
        m_messageModel->clear();
        emit currentConversationIdChanged();
        emit conversationAgentChanged();
        emit conversationCapabilitiesChanged();
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
    // Detect format from file extension
    QFileInfo fi(path);
    QString ext = fi.suffix().toLower();
    ExportHelper::ExportFormat fmt = ExportHelper::Markdown;
    if (ext == "html" || ext == "htm") fmt = ExportHelper::HTML;
    else if (ext == "pdf") fmt = ExportHelper::PDF;

    m_exportHelper->exportConversation(m_messageModel->toJsonArray(), path, fmt);
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

QJsonArray ChatManager::normalizeIdArray(const QJsonArray &ids) const {
    QSet<QString> seen;
    QJsonArray normalized;
    for (const auto &val : ids) {
        const QString id = val.toString().trimmed();
        if (id.isEmpty() || seen.contains(id)) {
            continue;
        }
        seen.insert(id);
        normalized.append(id);
    }
    return normalized;
}

QJsonArray ChatManager::effectiveSkillIds() const {
    if (!m_currentConversationId.isEmpty()) {
        const int idx = m_conversationModel->currentIndex();
        const QJsonObject convSettings = m_conversationModel->getConversationSettings(idx);
        QJsonArray ids = normalizeIdArray(convSettings.value("skillIds").toArray());
        if (!ids.isEmpty()) {
            return ids;
        }

        const QString agentId = conversationAgentId();
        if (!agentId.isEmpty()) {
            ids = normalizeIdArray(m_settings->getAgentById(agentId).value("skillIds").toArray());
            if (!ids.isEmpty()) {
                return ids;
            }
        }
        return {};
    }

    if (!m_pendingSkillIds.isEmpty()) {
        return normalizeIdArray(m_pendingSkillIds);
    }

    const QString agentId = conversationAgentId();
    if (!agentId.isEmpty()) {
        return normalizeIdArray(m_settings->getAgentById(agentId).value("skillIds").toArray());
    }
    return {};
}

QJsonArray ChatManager::effectiveMcpServerIds() const {
    if (!m_currentConversationId.isEmpty()) {
        const int idx = m_conversationModel->currentIndex();
        const QJsonObject convSettings = m_conversationModel->getConversationSettings(idx);
        QJsonArray ids = normalizeIdArray(convSettings.value("mcpServerIds").toArray());
        if (!ids.isEmpty()) {
            return ids;
        }

        const QString agentId = conversationAgentId();
        if (!agentId.isEmpty()) {
            ids = normalizeIdArray(m_settings->getAgentById(agentId).value("mcpServerIds").toArray());
            if (!ids.isEmpty()) {
                return ids;
            }
        }
        return {};
    }

    if (!m_pendingMcpServerIds.isEmpty()) {
        return normalizeIdArray(m_pendingMcpServerIds);
    }

    const QString agentId = conversationAgentId();
    if (!agentId.isEmpty()) {
        return normalizeIdArray(m_settings->getAgentById(agentId).value("mcpServerIds").toArray());
    }
    return {};
}

QJsonArray ChatManager::resolveSkills(const QJsonArray &ids) const {
    QJsonArray arr;
    for (const auto &val : normalizeIdArray(ids)) {
        const QJsonObject skill = m_settings->getSkillById(val.toString());
        if (!skill.isEmpty()) {
            arr.append(skill);
        }
    }
    return arr;
}

QJsonArray ChatManager::resolveMcpServers(const QJsonArray &ids) const {
    QJsonArray arr;
    for (const auto &val : normalizeIdArray(ids)) {
        const QJsonObject server = m_settings->getMcpServerById(val.toString());
        if (!server.isEmpty()) {
            arr.append(server);
        }
    }
    return arr;
}

QString ChatManager::buildCapabilityInstruction(const QJsonArray &skills, const QJsonArray &mcpServers) const {
    QStringList lines;
    if (!skills.isEmpty()) {
        lines << "Enabled skills:";
        for (const auto &item : skills) {
            if (!item.isObject()) continue;
            const QJsonObject obj = item.toObject();
            const QString name = obj.value("name").toString().trimmed();
            const QString prompt = obj.value("prompt").toString().trimmed();
            if (!name.isEmpty()) {
                lines << QString("- %1").arg(name);
            }
            if (!prompt.isEmpty()) {
                lines << QString("  skill_prompt: %1").arg(prompt);
            }
        }
    }
    if (!mcpServers.isEmpty()) {
        lines << "Enabled MCP servers:";
        for (const auto &item : mcpServers) {
            if (!item.isObject()) continue;
            const QJsonObject obj = item.toObject();
            const QString name = obj.value("name").toString().trimmed();
            const QString transport = obj.value("transport").toString().trimmed();
            const QString endpoint = obj.value("url").toString().trimmed();
            QString line = "- " + (name.isEmpty() ? QString("mcp") : name);
            if (!transport.isEmpty()) {
                line += " [" + transport + "]";
            }
            if (!endpoint.isEmpty()) {
                line += " " + endpoint;
            }
            lines << line;
        }
    }
    if (lines.isEmpty()) {
        return {};
    }
    return lines.join('\n');
}

QStringList ChatManager::availableModels() const {
    auto *provider = m_providers.value(currentProviderName(), nullptr);
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

QStringList ChatManager::providerNames() const {
    return {"OpenAI", "Claude", "Gemini", "DeepSeek", "Ollama", "Dify", "DeerFlow"};
}

bool ChatManager::isExternalProvider(const QString &provider) const {
    return provider == "Dify" || provider == "DeerFlow";
}

QJsonArray ChatManager::agents() const {
    return m_settings->agents();
}

QJsonObject ChatManager::agentById(const QString &id) const {
    return m_settings->getAgentById(id);
}

void ChatManager::saveAgent(const QJsonObject &agent) {
    m_settings->saveAgent(agent);
    emit agentsChanged();
    emit conversationCapabilitiesChanged();
}

void ChatManager::deleteAgent(const QString &id) {
    m_settings->deleteAgent(id);
    emit agentsChanged();
    emit conversationCapabilitiesChanged();
}

QJsonArray ChatManager::skills() const {
    return m_settings->skills();
}

QJsonObject ChatManager::skillById(const QString &id) const {
    return m_settings->getSkillById(id);
}

void ChatManager::saveSkill(const QJsonObject &skill) {
    m_settings->saveSkill(skill);
    emit skillsChanged();
    emit conversationCapabilitiesChanged();
}

void ChatManager::deleteSkill(const QString &id) {
    m_settings->deleteSkill(id);
    emit skillsChanged();
    emit conversationCapabilitiesChanged();
}

QJsonArray ChatManager::mcpServers() const {
    return m_settings->mcpServers();
}

QJsonObject ChatManager::mcpServerById(const QString &id) const {
    return m_settings->getMcpServerById(id);
}

void ChatManager::saveMcpServer(const QJsonObject &server) {
    m_settings->saveMcpServer(server);
    emit mcpServersChanged();
    emit conversationCapabilitiesChanged();
}

void ChatManager::deleteMcpServer(const QString &id) {
    m_settings->deleteMcpServer(id);
    emit mcpServersChanged();
    emit conversationCapabilitiesChanged();
}

QJsonArray ChatManager::conversationSkillIds() const {
    return effectiveSkillIds();
}

QJsonArray ChatManager::conversationMcpServerIds() const {
    return effectiveMcpServerIds();
}

void ChatManager::setConversationSkillIds(const QVariantList &ids) {
    QJsonArray incoming;
    for (const auto &val : ids) {
        incoming.append(QJsonValue::fromVariant(val));
    }
    const QJsonArray normalized = normalizeIdArray(incoming);
    m_pendingSkillIds = normalized;
    if (m_currentConversationId.isEmpty()) {
        emit conversationCapabilitiesChanged();
        return;
    }
    const int idx = m_conversationModel->currentIndex();
    if (idx < 0) {
        return;
    }
    m_conversationModel->updateConversationSettings(idx, QJsonObject{{"skillIds", normalized}});
    m_conversationModel->saveToFile(m_dataFilePath);
    emit conversationCapabilitiesChanged();
}

void ChatManager::setConversationMcpServerIds(const QVariantList &ids) {
    QJsonArray incoming;
    for (const auto &val : ids) {
        incoming.append(QJsonValue::fromVariant(val));
    }
    const QJsonArray normalized = normalizeIdArray(incoming);
    m_pendingMcpServerIds = normalized;
    if (m_currentConversationId.isEmpty()) {
        emit conversationCapabilitiesChanged();
        return;
    }
    const int idx = m_conversationModel->currentIndex();
    if (idx < 0) {
        return;
    }
    m_conversationModel->updateConversationSettings(idx, QJsonObject{{"mcpServerIds", normalized}});
    m_conversationModel->saveToFile(m_dataFilePath);
    emit conversationCapabilitiesChanged();
}

QStringList ChatManager::conversationSkillNames() const {
    QStringList names;
    for (const auto &val : effectiveSkillIds()) {
        const QJsonObject obj = m_settings->getSkillById(val.toString());
        QString name = obj.value("name").toString().trimmed();
        if (name.isEmpty()) {
            name = val.toString();
        }
        if (!name.isEmpty()) {
            names.append(name);
        }
    }
    return names;
}

QStringList ChatManager::conversationMcpServerNames() const {
    QStringList names;
    for (const auto &val : effectiveMcpServerIds()) {
        const QJsonObject obj = m_settings->getMcpServerById(val.toString());
        QString name = obj.value("name").toString().trimmed();
        if (name.isEmpty()) {
            name = val.toString();
        }
        if (!name.isEmpty()) {
            names.append(name);
        }
    }
    return names;
}

QString ChatManager::conversationAgentId() const {
    if (!m_currentConversationId.isEmpty()) {
        return m_conversationModel->getConversationAgentById(m_currentConversationId).trimmed();
    }
    return m_pendingAgentId.trimmed();
}

QString ChatManager::conversationAgentName() const {
    const QString id = conversationAgentId();
    if (id.isEmpty()) {
        return {};
    }
    const QJsonObject agent = m_settings->getAgentById(id);
    const QString name = agent.value("name").toString().trimmed();
    return name.isEmpty() ? id : name;
}

void ChatManager::setConversationAgentId(const QString &agentId) {
    const QString normalizedAgentId = agentId.trimmed();
    const QString previousAgentId = conversationAgentId();
    m_pendingAgentId = normalizedAgentId;
    if (!normalizedAgentId.isEmpty()) {
        const QJsonObject pendingAgent = m_settings->getAgentById(normalizedAgentId);
        m_pendingSkillIds = normalizeIdArray(pendingAgent.value("skillIds").toArray());
        m_pendingMcpServerIds = normalizeIdArray(pendingAgent.value("mcpServerIds").toArray());
    }

    if (m_currentConversationId.isEmpty()) {
        if (previousAgentId != normalizedAgentId) {
            emit conversationAgentChanged();
            emit conversationCapabilitiesChanged();
        }
        return;
    }

    const int index = m_conversationModel->currentIndex();
    if (index < 0) {
        return;
    }

    const QString oldAgentId = m_conversationModel->getConversationAgentById(m_currentConversationId).trimmed();

    QJsonObject settings;
    settings["agentId"] = normalizedAgentId;
    if (!normalizedAgentId.isEmpty()) {
        const QJsonObject agent = m_settings->getAgentById(normalizedAgentId);
        const QString provider = agent.value("provider").toString().trimmed();
        if (!provider.isEmpty()) {
            settings["provider"] = provider;
        }
        settings["skillIds"] = normalizeIdArray(agent.value("skillIds").toArray());
        settings["mcpServerIds"] = normalizeIdArray(agent.value("mcpServerIds").toArray());
    } else {
        settings["skillIds"] = QJsonArray{};
        settings["mcpServerIds"] = QJsonArray{};
    }
    m_conversationModel->updateConversationSettings(index, settings);

    if (oldAgentId != normalizedAgentId) {
        bool shouldResetRuntime = true;
        if (oldAgentId.isEmpty() && !normalizedAgentId.isEmpty()) {
            // Keep runtime when first binding an agent onto an existing external session.
            shouldResetRuntime = false;
        }

        const QString providerName = currentProviderName();
        if (shouldResetRuntime && isExternalProvider(providerName)) {
            m_conversationModel->updateConversationRuntime(index, QJsonObject{
                {"conversation_id", ""},
                {"last_message_id", ""},
                {"run_id", ""},
            });
        }
    }

    m_conversationModel->saveToFile(m_dataFilePath);
    if (previousAgentId != normalizedAgentId) {
        emit conversationAgentChanged();
        emit conversationCapabilitiesChanged();
    }
}
