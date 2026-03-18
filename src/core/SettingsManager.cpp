#include "SettingsManager.h"
#include <QDir>
#include <QJsonDocument>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("QChat", "QChat")
{
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_dataPath);
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
    settings.endGroup();
    return obj;
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

QString SettingsManager::dataPath() const { return m_dataPath; }
