#include "SettingsManager.h"
#include <QDir>
#include <QJsonDocument>
#include <QUuid>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("QChat", "QChat")
{
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_dataPath);
    ensureBuiltinData();
}

QString SettingsManager::theme() const { return m_settings.value("theme", "dark").toString(); }
void SettingsManager::setTheme(const QString &theme) {
    if (this->theme() != theme) { m_settings.setValue("theme", theme); emit themeChanged(); }
}

double SettingsManager::fontSize() const { return m_settings.value("fontSize", 14.0).toDouble(); }
void SettingsManager::setFontSize(double size) {
    if (fontSize() != size) { m_settings.setValue("fontSize", size); emit fontSizeChanged(); }
}

QString SettingsManager::language() const { return m_settings.value("language", "en").toString(); }
void SettingsManager::setLanguage(const QString &lang) {
    if (language() != lang) { m_settings.setValue("language", lang); emit languageChanged(); }
}

bool SettingsManager::sendOnEnter() const { return m_settings.value("sendOnEnter", true).toBool(); }
void SettingsManager::setSendOnEnter(bool val) {
    if (sendOnEnter() != val) { m_settings.setValue("sendOnEnter", val); emit sendOnEnterChanged(); }
}

bool SettingsManager::mathRenderer() const { return m_settings.value("mathRenderer", false).toBool(); }
void SettingsManager::setMathRenderer(bool val) {
    if (mathRenderer() != val) { m_settings.setValue("mathRenderer", val); emit mathRendererChanged(); }
}

bool SettingsManager::mermaidEnabled() const { return m_settings.value("mermaidEnabled", false).toBool(); }
void SettingsManager::setMermaidEnabled(bool val) {
    if (mermaidEnabled() != val) { m_settings.setValue("mermaidEnabled", val); emit mermaidEnabledChanged(); }
}

int SettingsManager::historyMessageCount() const { return m_settings.value("historyMessageCount", 20).toInt(); }
void SettingsManager::setHistoryMessageCount(int count) {
    if (historyMessageCount() != count) { m_settings.setValue("historyMessageCount", count); emit historyMessageCountChanged(); }
}

QString SettingsManager::chatMode() const { return m_settings.value("chatMode", "quick").toString(); }
void SettingsManager::setChatMode(const QString &mode) {
    if (chatMode() != mode) { m_settings.setValue("chatMode", mode); emit chatModeChanged(); }
}

bool SettingsManager::deepResearch() const { return m_settings.value("deepResearch", false).toBool(); }
void SettingsManager::setDeepResearch(bool val) {
    if (deepResearch() != val) { m_settings.setValue("deepResearch", val); emit deepResearchChanged(); }
}

bool SettingsManager::markdownRendering() const { return m_settings.value("markdownRendering", true).toBool(); }
void SettingsManager::setMarkdownRendering(bool val) {
    if (markdownRendering() != val) { m_settings.setValue("markdownRendering", val); emit markdownRenderingChanged(); }
}

QString SettingsManager::systemPrompt() const { return m_settings.value("systemPrompt", "").toString(); }
void SettingsManager::setSystemPrompt(const QString &prompt) {
    if (systemPrompt() != prompt) { m_settings.setValue("systemPrompt", prompt); emit systemPromptChanged(); }
}

double SettingsManager::temperature() const { return m_settings.value("temperature", 0.7).toDouble(); }
void SettingsManager::setTemperature(double temp) {
    if (temperature() != temp) { m_settings.setValue("temperature", temp); emit temperatureChanged(); }
}

int SettingsManager::maxTokens() const { return m_settings.value("maxTokens", 4096).toInt(); }
void SettingsManager::setMaxTokens(int tokens) {
    if (maxTokens() != tokens) { m_settings.setValue("maxTokens", tokens); emit maxTokensChanged(); }
}

bool SettingsManager::enableThinking() const { return m_settings.value("enableThinking", false).toBool(); }
void SettingsManager::setEnableThinking(bool enabled) {
    if (enableThinking() != enabled) { m_settings.setValue("enableThinking", enabled); emit enableThinkingChanged(); }
}

QString SettingsManager::currentProvider() const { return m_settings.value("currentProvider", "OpenAI").toString(); }
void SettingsManager::setCurrentProvider(const QString &provider) {
    if (currentProvider() != provider) { m_settings.setValue("currentProvider", provider); emit currentProviderChanged(); }
}

QString SettingsManager::currentModel() const { return m_settings.value("currentModel", "gpt-4o").toString(); }
void SettingsManager::setCurrentModel(const QString &model) {
    if (currentModel() != model) { m_settings.setValue("currentModel", model); emit currentModelChanged(); }
}

void SettingsManager::saveProviderConfig(const QString &name, const QString &apiKey,
                                          const QString &baseUrl, const QString &defaultModel) {
    m_settings.beginGroup("providers/" + name);
    m_settings.setValue("apiKey", apiKey);
    m_settings.setValue("baseUrl", baseUrl);
    m_settings.setValue("defaultModel", defaultModel);
    m_settings.endGroup();
}

QJsonObject SettingsManager::getProviderConfig(const QString &name) const {
    QSettings &settings = const_cast<QSettings &>(m_settings);
    settings.beginGroup("providers/" + name);
    QJsonObject obj;
    obj["apiKey"] = settings.value("apiKey").toString();
    obj["baseUrl"] = settings.value("baseUrl").toString();
    obj["defaultModel"] = settings.value("defaultModel").toString();
    obj["extra"] = QJsonObject::fromVariantMap(settings.value("extra").toMap());
    settings.endGroup();
    return obj;
}

void SettingsManager::saveProviderExtra(const QString &name, const QJsonObject &extra) {
    m_settings.beginGroup("providers/" + name);
    m_settings.setValue("extra", extra.toVariantMap());
    m_settings.endGroup();
}

QJsonObject SettingsManager::getProviderExtra(const QString &name) const {
    QSettings &settings = const_cast<QSettings &>(m_settings);
    settings.beginGroup("providers/" + name);
    QJsonObject extra = QJsonObject::fromVariantMap(settings.value("extra").toMap());
    settings.endGroup();
    return extra;
}

QStringList SettingsManager::providerNames() const {
    QSettings &settings = const_cast<QSettings &>(m_settings);
    settings.beginGroup("providers");
    QStringList names = settings.childGroups();
    settings.endGroup();
    return names;
}

void SettingsManager::removeProvider(const QString &name) {
    m_settings.remove("providers/" + name);
}

QJsonArray SettingsManager::loadAgentsInternal() const {
    return loadCollection("agents");
}

void SettingsManager::saveAgentsInternal(const QJsonArray &agents) {
    saveCollection("agents", agents);
}

QJsonArray SettingsManager::loadCollection(const QString &key) const {
    const QByteArray raw = m_settings.value(key, "[]").toByteArray();
    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    return doc.isArray() ? doc.array() : QJsonArray{};
}

void SettingsManager::saveCollection(const QString &key, const QJsonArray &list) {
    m_settings.setValue(key, QJsonDocument(list).toJson(QJsonDocument::Compact));
}

void SettingsManager::ensureBuiltinData() {
    QJsonArray list = loadAgentsInternal();
    bool hasMeetingAssistant = false;
    for (const auto &item : list) {
        if (!item.isObject()) continue;
        if (item.toObject().value("id").toString() == "builtin-meeting-minutes") {
            hasMeetingAssistant = true;
            break;
        }
    }

    if (!hasMeetingAssistant) {
        QJsonObject agent;
        agent["id"] = "builtin-meeting-minutes";
        agent["name"] = "会议纪要助手";
        agent["provider"] = "Dify";
        agent["baseUrl"] = "";
        agent["apiKey"] = "";
        agent["readonly"] = true;
        agent["builtin"] = true;
        agent["description"] = "预置会议纪要智能体，可配置为 Dify 或 DeerFlow 后端。";
        agent["skillIds"] = QJsonArray{QJsonValue(QStringLiteral("builtin-deep-research"))};
        agent["mcpServerIds"] = QJsonArray{};
        list.append(agent);
        saveAgentsInternal(list);
    }

    QJsonArray skillList = loadCollection("skills");
    bool hasDeepResearchSkill = false;
    for (const auto &item : skillList) {
        if (!item.isObject()) continue;
        if (item.toObject().value("id").toString() == "builtin-deep-research") {
            hasDeepResearchSkill = true;
            break;
        }
    }
    if (!hasDeepResearchSkill) {
        QJsonObject skill;
        skill["id"] = "builtin-deep-research";
        skill["name"] = "深度调研";
        skill["description"] = "分步规划、交叉验证、最后给出结论和证据。";
        skill["prompt"] = "你处于深度调研模式。先给出调研计划，再执行与验证，最后输出结论、依据、风险与后续建议。";
        skill["readonly"] = true;
        skill["builtin"] = true;
        skillList.append(skill);
        saveCollection("skills", skillList);
    }

    QJsonArray mcpList = loadCollection("mcpServers");
    bool hasDefaultMcp = false;
    for (const auto &item : mcpList) {
        if (!item.isObject()) continue;
        if (item.toObject().value("id").toString() == "builtin-default-mcp") {
            hasDefaultMcp = true;
            break;
        }
    }
    if (!hasDefaultMcp) {
        QJsonObject mcp;
        mcp["id"] = "builtin-default-mcp";
        mcp["name"] = "默认MCP";
        mcp["transport"] = "sse";
        mcp["url"] = "";
        mcp["command"] = "";
        mcp["args"] = "";
        mcp["envJson"] = "{}";
        mcp["description"] = "示例 MCP 配置，请按实际服务地址修改。";
        mcp["enabled"] = true;
        mcp["readonly"] = false;
        mcp["builtin"] = true;
        mcpList.append(mcp);
        saveCollection("mcpServers", mcpList);
    }
}

QJsonArray SettingsManager::agents() const {
    return loadAgentsInternal();
}

QJsonObject SettingsManager::getAgentById(const QString &id) const {
    if (id.trimmed().isEmpty()) {
        return {};
    }
    const QJsonArray list = loadAgentsInternal();
    for (const auto &item : list) {
        if (!item.isObject()) continue;
        const QJsonObject obj = item.toObject();
        if (obj.value("id").toString() == id) {
            return obj;
        }
    }
    return {};
}

void SettingsManager::saveAgent(const QJsonObject &agent) {
    QJsonObject normalized = agent;
    QString id = normalized.value("id").toString().trimmed();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        normalized["id"] = id;
    }
    if (normalized.value("name").toString().trimmed().isEmpty()) {
        normalized["name"] = "New Agent";
    }
    if (normalized.value("provider").toString().trimmed().isEmpty()) {
        normalized["provider"] = "Dify";
    }
    if (!normalized.contains("skillIds") || !normalized.value("skillIds").isArray()) {
        normalized["skillIds"] = QJsonArray{};
    }
    if (!normalized.contains("mcpServerIds") || !normalized.value("mcpServerIds").isArray()) {
        normalized["mcpServerIds"] = QJsonArray{};
    }

    QJsonArray list = loadAgentsInternal();
    bool replaced = false;
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        QJsonObject current = list.at(i).toObject();
        if (current.value("id").toString() == id) {
            if (current.value("readonly").toBool()) {
                normalized["readonly"] = true;
                normalized["builtin"] = current.value("builtin").toBool();
            }
            list[i] = normalized;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        list.append(normalized);
    }
    saveAgentsInternal(list);
}

void SettingsManager::deleteAgent(const QString &id) {
    if (id.trimmed().isEmpty()) {
        return;
    }
    QJsonArray list = loadAgentsInternal();
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        const QJsonObject obj = list.at(i).toObject();
        if (obj.value("id").toString() == id) {
            if (obj.value("readonly").toBool()) {
                return;
            }
            list.removeAt(i);
            break;
        }
    }
    saveAgentsInternal(list);
}

QJsonArray SettingsManager::skills() const {
    return loadCollection("skills");
}

QJsonObject SettingsManager::getSkillById(const QString &id) const {
    if (id.trimmed().isEmpty()) {
        return {};
    }
    const QJsonArray list = loadCollection("skills");
    for (const auto &item : list) {
        if (!item.isObject()) continue;
        const QJsonObject obj = item.toObject();
        if (obj.value("id").toString() == id) {
            return obj;
        }
    }
    return {};
}

void SettingsManager::saveSkill(const QJsonObject &skill) {
    QJsonObject normalized = skill;
    QString id = normalized.value("id").toString().trimmed();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        normalized["id"] = id;
    }
    if (normalized.value("name").toString().trimmed().isEmpty()) {
        normalized["name"] = "New Skill";
    }
    if (!normalized.contains("prompt")) {
        normalized["prompt"] = "";
    }

    QJsonArray list = loadCollection("skills");
    bool replaced = false;
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        QJsonObject current = list.at(i).toObject();
        if (current.value("id").toString() == id) {
            if (current.value("readonly").toBool()) {
                normalized["readonly"] = true;
                normalized["builtin"] = current.value("builtin").toBool();
            }
            list[i] = normalized;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        list.append(normalized);
    }
    saveCollection("skills", list);
}

void SettingsManager::deleteSkill(const QString &id) {
    if (id.trimmed().isEmpty()) {
        return;
    }
    QJsonArray list = loadCollection("skills");
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        const QJsonObject obj = list.at(i).toObject();
        if (obj.value("id").toString() == id) {
            if (obj.value("readonly").toBool()) {
                return;
            }
            list.removeAt(i);
            break;
        }
    }
    saveCollection("skills", list);
}

QJsonArray SettingsManager::mcpServers() const {
    return loadCollection("mcpServers");
}

QJsonObject SettingsManager::getMcpServerById(const QString &id) const {
    if (id.trimmed().isEmpty()) {
        return {};
    }
    const QJsonArray list = loadCollection("mcpServers");
    for (const auto &item : list) {
        if (!item.isObject()) continue;
        const QJsonObject obj = item.toObject();
        if (obj.value("id").toString() == id) {
            return obj;
        }
    }
    return {};
}

void SettingsManager::saveMcpServer(const QJsonObject &server) {
    QJsonObject normalized = server;
    QString id = normalized.value("id").toString().trimmed();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        normalized["id"] = id;
    }
    if (normalized.value("name").toString().trimmed().isEmpty()) {
        normalized["name"] = "New MCP";
    }
    if (normalized.value("transport").toString().trimmed().isEmpty()) {
        normalized["transport"] = "sse";
    }
    if (!normalized.contains("enabled")) {
        normalized["enabled"] = true;
    }

    QJsonArray list = loadCollection("mcpServers");
    bool replaced = false;
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        QJsonObject current = list.at(i).toObject();
        if (current.value("id").toString() == id) {
            if (current.value("readonly").toBool()) {
                normalized["readonly"] = true;
                normalized["builtin"] = current.value("builtin").toBool();
            }
            list[i] = normalized;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        list.append(normalized);
    }
    saveCollection("mcpServers", list);
}

void SettingsManager::deleteMcpServer(const QString &id) {
    if (id.trimmed().isEmpty()) {
        return;
    }
    QJsonArray list = loadCollection("mcpServers");
    for (int i = 0; i < list.size(); ++i) {
        if (!list.at(i).isObject()) continue;
        const QJsonObject obj = list.at(i).toObject();
        if (obj.value("id").toString() == id) {
            if (obj.value("readonly").toBool()) {
                return;
            }
            list.removeAt(i);
            break;
        }
    }
    saveCollection("mcpServers", list);
}

QString SettingsManager::dataPath() const { return m_dataPath; }
