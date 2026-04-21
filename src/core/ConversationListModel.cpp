#include "ConversationListModel.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>

ConversationListModel::ConversationListModel(QObject *parent) : QAbstractListModel(parent) {}

int ConversationListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_conversations.count();
}

QVariant ConversationListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_conversations.count())
        return {};

    const Conversation &conv = m_conversations[index.row()];
    switch (role) {
    case IdRole: return conv.id;
    case TitleRole: return conv.title;
    case LastMessageRole: return conv.lastMessage;
    case TimestampRole: return conv.timestamp;
    case ModelRole: return conv.model;
    case MessageCountRole: return conv.messageCount;
    }
    return {};
}

QHash<int, QByteArray> ConversationListModel::roleNames() const {
    return {
        {IdRole, "conversationId"},
        {TitleRole, "title"},
        {LastMessageRole, "lastMessage"},
        {TimestampRole, "timestamp"},
        {ModelRole, "model"},
        {MessageCountRole, "messageCount"},
    };
}

void ConversationListModel::setCurrentIndex(int idx) {
    if (m_currentIndex != idx) {
        m_currentIndex = idx;
        emit currentIndexChanged();
        if (idx >= 0 && idx < m_conversations.count()) {
            const auto &conv = m_conversations[idx];
            emit conversationSelected(conv.id, conv.messages);
        }
    }
}

QString ConversationListModel::createConversation(const QString &title) {
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    Conversation conv;
    conv.id = id;
    conv.title = title;
    conv.timestamp = QDateTime::currentDateTime();

    beginInsertRows(QModelIndex(), 0, 0);
    m_conversations.prepend(conv);
    endInsertRows();

    setCurrentIndex(0);
    emit countChanged();
    return id;
}

void ConversationListModel::deleteConversation(int index) {
    if (index < 0 || index >= m_conversations.count()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_conversations.removeAt(index);
    endRemoveRows();

    if (m_currentIndex >= m_conversations.count())
        setCurrentIndex(m_conversations.count() - 1);

    emit countChanged();
}

void ConversationListModel::renameConversation(int index, const QString &title) {
    if (index < 0 || index >= m_conversations.count()) return;
    m_conversations[index].title = title;
    QModelIndex idx = this->index(index);
    emit dataChanged(idx, idx, {TitleRole});
}

void ConversationListModel::updateConversation(const QString &id, const QJsonArray &messages,
                                                 const QString &lastMsg, const QString &model) {
    for (int i = 0; i < m_conversations.count(); ++i) {
        if (m_conversations[i].id == id) {
            m_conversations[i].messages = messages;
            m_conversations[i].lastMessage = lastMsg;
            m_conversations[i].model = model;
            m_conversations[i].messageCount = messages.count();
            m_conversations[i].timestamp = QDateTime::currentDateTime();

            // Auto-generate title from first user message
            if (m_conversations[i].title == "New Chat" && messages.count() > 0) {
                for (const auto &msg : messages) {
                    if (msg.toObject()["role"].toString() == "user") {
                        QString t = msg.toObject()["content"].toString().left(40);
                        if (t.length() >= 40) t += "...";
                        m_conversations[i].title = t;
                        break;
                    }
                }
            }

            QModelIndex idx = this->index(i);
            emit dataChanged(idx, idx);
            return;
        }
    }
}

QJsonObject ConversationListModel::getConversationSettings(int index) const {
    if (index < 0 || index >= m_conversations.count()) return {};
    const Conversation &conv = m_conversations[index];
    QJsonObject obj;
    obj["title"] = conv.title;
    obj["provider"] = conv.provider;
    obj["agentId"] = conv.agentId;
    obj["systemPrompt"] = conv.systemPrompt;
    obj["temperature"] = conv.temperature;
    obj["parameters"] = conv.parameters;
    obj["markdownEnabled"] = conv.markdownEnabled;
    obj["historyToolEnabled"] = conv.historyToolEnabled;
    obj["skillIds"] = conv.skillIds;
    obj["mcpServerIds"] = conv.mcpServerIds;
    return obj;
}

void ConversationListModel::updateConversationSettings(int index, const QJsonObject &settings) {
    if (index < 0 || index >= m_conversations.count()) return;
    Conversation &conv = m_conversations[index];
    if (settings.contains("title")) conv.title = settings["title"].toString();
    if (settings.contains("provider")) conv.provider = settings["provider"].toString();
    if (settings.contains("agentId")) conv.agentId = settings["agentId"].toString();
    if (settings.contains("systemPrompt")) conv.systemPrompt = settings["systemPrompt"].toString();
    if (settings.contains("temperature")) conv.temperature = settings["temperature"].toDouble();
    if (settings.contains("parameters")) conv.parameters = settings["parameters"].toString();
    if (settings.contains("markdownEnabled")) conv.markdownEnabled = settings["markdownEnabled"].toBool();
    if (settings.contains("historyToolEnabled")) conv.historyToolEnabled = settings["historyToolEnabled"].toBool();
    if (settings.contains("skillIds")) conv.skillIds = settings["skillIds"].toArray();
    if (settings.contains("mcpServerIds")) conv.mcpServerIds = settings["mcpServerIds"].toArray();
    QModelIndex idx = this->index(index);
    emit dataChanged(idx, idx);
}

QJsonObject ConversationListModel::getConversationRuntime(int index) const {
    if (index < 0 || index >= m_conversations.count()) return {};
    const Conversation &conv = m_conversations[index];
    QJsonObject obj;
    obj["conversation_id"] = conv.externalConversationId;
    obj["last_message_id"] = conv.externalMessageId;
    obj["run_id"] = conv.externalRunId;
    return obj;
}

void ConversationListModel::updateConversationRuntime(int index, const QJsonObject &runtime) {
    if (index < 0 || index >= m_conversations.count()) return;
    Conversation &conv = m_conversations[index];
    if (runtime.contains("conversation_id")) conv.externalConversationId = runtime["conversation_id"].toString();
    if (runtime.contains("last_message_id")) conv.externalMessageId = runtime["last_message_id"].toString();
    if (runtime.contains("run_id")) conv.externalRunId = runtime["run_id"].toString();
    QModelIndex idx = this->index(index);
    emit dataChanged(idx, idx);
}

QString ConversationListModel::getConversationProviderById(const QString &id) const {
    for (const auto &conv : m_conversations) {
        if (conv.id == id) {
            return conv.provider;
        }
    }
    return {};
}

QString ConversationListModel::getConversationAgentById(const QString &id) const {
    for (const auto &conv : m_conversations) {
        if (conv.id == id) {
            return conv.agentId;
        }
    }
    return {};
}

QJsonObject ConversationListModel::getConversationRuntimeById(const QString &id) const {
    for (const auto &conv : m_conversations) {
        if (conv.id == id) {
            QJsonObject obj;
            obj["conversation_id"] = conv.externalConversationId;
            obj["last_message_id"] = conv.externalMessageId;
            obj["run_id"] = conv.externalRunId;
            return obj;
        }
    }
    return {};
}

void ConversationListModel::updateConversationRuntimeById(const QString &id, const QJsonObject &runtime) {
    for (int i = 0; i < m_conversations.count(); ++i) {
        if (m_conversations[i].id == id) {
            updateConversationRuntime(i, runtime);
            return;
        }
    }
}

QJsonArray ConversationListModel::getMessages(const QString &id) const {
    for (const auto &conv : m_conversations) {
        if (conv.id == id) return conv.messages;
    }
    return {};
}

void ConversationListModel::saveToFile(const QString &path) {
    QJsonArray arr;
    for (const auto &conv : m_conversations) {
        QJsonObject obj;
        obj["id"] = conv.id;
        obj["title"] = conv.title;
        obj["lastMessage"] = conv.lastMessage;
        obj["timestamp"] = conv.timestamp.toString(Qt::ISODate);
        obj["model"] = conv.model;
        obj["messages"] = conv.messages;
        obj["provider"] = conv.provider;
        obj["agentId"] = conv.agentId;
        obj["systemPrompt"] = conv.systemPrompt;
        obj["temperature"] = conv.temperature;
        obj["parameters"] = conv.parameters;
        obj["markdownEnabled"] = conv.markdownEnabled;
        obj["historyToolEnabled"] = conv.historyToolEnabled;
        obj["skillIds"] = conv.skillIds;
        obj["mcpServerIds"] = conv.mcpServerIds;
        obj["externalConversationId"] = conv.externalConversationId;
        obj["externalMessageId"] = conv.externalMessageId;
        obj["externalRunId"] = conv.externalRunId;
        arr.append(obj);
    }

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
    }
}

void ConversationListModel::loadFromFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return;

    beginResetModel();
    m_conversations.clear();
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();
        Conversation conv;
        conv.id = obj["id"].toString();
        conv.title = obj["title"].toString();
        conv.lastMessage = obj["lastMessage"].toString();
        conv.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        conv.model = obj["model"].toString();
        conv.messages = obj["messages"].toArray();
        conv.messageCount = conv.messages.count();
        conv.provider = obj["provider"].toString();
        conv.agentId = obj["agentId"].toString();
        conv.systemPrompt = obj["systemPrompt"].toString();
        conv.temperature = obj.contains("temperature") ? obj["temperature"].toDouble() : -1.0;
        conv.parameters = obj["parameters"].toString();
        conv.markdownEnabled = obj.contains("markdownEnabled") ? obj["markdownEnabled"].toBool() : true;
        conv.historyToolEnabled = obj.contains("historyToolEnabled") ? obj["historyToolEnabled"].toBool() : true;
        conv.skillIds = obj.contains("skillIds") && obj["skillIds"].isArray() ? obj["skillIds"].toArray() : QJsonArray{};
        conv.mcpServerIds = obj.contains("mcpServerIds") && obj["mcpServerIds"].isArray() ? obj["mcpServerIds"].toArray() : QJsonArray{};
        conv.externalConversationId = obj["externalConversationId"].toString();
        conv.externalMessageId = obj["externalMessageId"].toString();
        conv.externalRunId = obj["externalRunId"].toString();
        m_conversations.append(conv);
    }
    endResetModel();
    emit countChanged();
}
