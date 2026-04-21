#pragma once

#include <QObject>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

class SettingsManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(double fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool sendOnEnter READ sendOnEnter WRITE setSendOnEnter NOTIFY sendOnEnterChanged)
    Q_PROPERTY(QString systemPrompt READ systemPrompt WRITE setSystemPrompt NOTIFY systemPromptChanged)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(int maxTokens READ maxTokens WRITE setMaxTokens NOTIFY maxTokensChanged)
    Q_PROPERTY(bool enableThinking READ enableThinking WRITE setEnableThinking NOTIFY enableThinkingChanged)
    Q_PROPERTY(QString currentProvider READ currentProvider WRITE setCurrentProvider NOTIFY currentProviderChanged)
    Q_PROPERTY(QString currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QString dataPath READ dataPath CONSTANT)
    // New settings
    Q_PROPERTY(bool mathRenderer READ mathRenderer WRITE setMathRenderer NOTIFY mathRendererChanged)
    Q_PROPERTY(bool mermaidEnabled READ mermaidEnabled WRITE setMermaidEnabled NOTIFY mermaidEnabledChanged)
    Q_PROPERTY(int historyMessageCount READ historyMessageCount WRITE setHistoryMessageCount NOTIFY historyMessageCountChanged)
    Q_PROPERTY(QString chatMode READ chatMode WRITE setChatMode NOTIFY chatModeChanged)
    Q_PROPERTY(bool deepResearch READ deepResearch WRITE setDeepResearch NOTIFY deepResearchChanged)
    Q_PROPERTY(bool markdownRendering READ markdownRendering WRITE setMarkdownRendering NOTIFY markdownRenderingChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    // Theme & UI
    QString theme() const;
    void setTheme(const QString &theme);
    double fontSize() const;
    void setFontSize(double size);
    QString language() const;
    void setLanguage(const QString &lang);
    bool sendOnEnter() const;
    void setSendOnEnter(bool val);

    // New settings
    bool mathRenderer() const;
    void setMathRenderer(bool val);
    bool mermaidEnabled() const;
    void setMermaidEnabled(bool val);
    int historyMessageCount() const;
    void setHistoryMessageCount(int count);
    QString chatMode() const;
    void setChatMode(const QString &mode);
    bool deepResearch() const;
    void setDeepResearch(bool val);
    bool markdownRendering() const;
    void setMarkdownRendering(bool val);

    // Chat Settings
    QString systemPrompt() const;
    void setSystemPrompt(const QString &prompt);
    double temperature() const;
    void setTemperature(double temp);
    int maxTokens() const;
    void setMaxTokens(int tokens);
    bool enableThinking() const;
    void setEnableThinking(bool enabled);

    // Provider Settings
    QString currentProvider() const;
    void setCurrentProvider(const QString &provider);
    QString currentModel() const;
    void setCurrentModel(const QString &model);

    Q_INVOKABLE void saveProviderConfig(const QString &name, const QString &apiKey,
                                         const QString &baseUrl, const QString &defaultModel);
    Q_INVOKABLE QJsonObject getProviderConfig(const QString &name) const;
    Q_INVOKABLE void saveProviderExtra(const QString &name, const QJsonObject &extra);
    Q_INVOKABLE QJsonObject getProviderExtra(const QString &name) const;
    Q_INVOKABLE QStringList providerNames() const;
    Q_INVOKABLE void removeProvider(const QString &name);
    Q_INVOKABLE QJsonArray agents() const;
    Q_INVOKABLE QJsonObject getAgentById(const QString &id) const;
    Q_INVOKABLE void saveAgent(const QJsonObject &agent);
    Q_INVOKABLE void deleteAgent(const QString &id);
    Q_INVOKABLE QJsonArray skills() const;
    Q_INVOKABLE QJsonObject getSkillById(const QString &id) const;
    Q_INVOKABLE void saveSkill(const QJsonObject &skill);
    Q_INVOKABLE void deleteSkill(const QString &id);
    Q_INVOKABLE QJsonArray mcpServers() const;
    Q_INVOKABLE QJsonObject getMcpServerById(const QString &id) const;
    Q_INVOKABLE void saveMcpServer(const QJsonObject &server);
    Q_INVOKABLE void deleteMcpServer(const QString &id);

    QString dataPath() const;

signals:
    void themeChanged();
    void fontSizeChanged();
    void languageChanged();
    void sendOnEnterChanged();
    void systemPromptChanged();
    void temperatureChanged();
    void maxTokensChanged();
    void enableThinkingChanged();
    void currentProviderChanged();
    void currentModelChanged();
    void mathRendererChanged();
    void mermaidEnabledChanged();
    void historyMessageCountChanged();
    void chatModeChanged();
    void deepResearchChanged();
    void markdownRenderingChanged();

private:
    QJsonArray loadCollection(const QString &key) const;
    void saveCollection(const QString &key, const QJsonArray &list);
    QJsonArray loadAgentsInternal() const;
    void saveAgentsInternal(const QJsonArray &agents);
    void ensureBuiltinData();

    QSettings m_settings;
    QString m_dataPath;
};
