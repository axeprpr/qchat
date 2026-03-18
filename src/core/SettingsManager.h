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
    Q_INVOKABLE QStringList providerNames() const;
    Q_INVOKABLE void removeProvider(const QString &name);

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

private:
    QSettings m_settings;
    QString m_dataPath;
};
