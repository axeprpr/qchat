#include "MessageModel.h"
#include <QJsonObject>
#include <QJsonArray>

MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent) {}

int MessageModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_messages.count();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_messages.count())
        return {};

    const Message &msg = m_messages[index.row()];
    switch (role) {
    case RoleRole: return msg.role;
    case ContentRole: return msg.content;
    case ThinkingRole: return msg.thinkingContent;
    case TimestampRole: return msg.timestamp;
    case IsStreamingRole: return msg.isStreaming;
    case AttachmentsRole: return msg.attachments;
    case IsErrorRole: return msg.isError;
    }
    return {};
}

QHash<int, QByteArray> MessageModel::roleNames() const {
    return {
        {RoleRole, "role"},
        {ContentRole, "content"},
        {ThinkingRole, "thinkingContent"},
        {TimestampRole, "timestamp"},
        {IsStreamingRole, "isStreaming"},
        {AttachmentsRole, "attachments"},
        {IsErrorRole, "isError"},
    };
}

void MessageModel::addMessage(const QString &role, const QString &content,
                                const QStringList &attachments) {
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append({role, content, "", QDateTime::currentDateTime(), false, attachments, false});
    endInsertRows();
    emit countChanged();
}

void MessageModel::addErrorMessage(const QString &error) {
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append({"assistant", error, "", QDateTime::currentDateTime(), false, {}, true});
    endInsertRows();
    emit countChanged();
}

void MessageModel::startAssistantMessage() {
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append({"assistant", "", "", QDateTime::currentDateTime(), true, {}, false});
    endInsertRows();
    emit countChanged();
}

void MessageModel::appendToLastMessage(const QString &text) {
    if (m_messages.isEmpty()) return;
    int lastIdx = m_messages.count() - 1;
    m_messages[lastIdx].content += text;
    QModelIndex idx = index(lastIdx);
    emit dataChanged(idx, idx, {ContentRole});
    emit streamingUpdated();
}

void MessageModel::appendThinking(const QString &text) {
    if (m_messages.isEmpty()) return;
    int lastIdx = m_messages.count() - 1;
    m_messages[lastIdx].thinkingContent += text;
    QModelIndex idx = index(lastIdx);
    emit dataChanged(idx, idx, {ThinkingRole});
    emit streamingUpdated();
}

void MessageModel::finishStreaming() {
    if (m_messages.isEmpty()) return;
    int lastIdx = m_messages.count() - 1;
    m_messages[lastIdx].isStreaming = false;
    m_messages[lastIdx].timestamp = QDateTime::currentDateTime();
    QModelIndex idx = index(lastIdx);
    emit dataChanged(idx, idx, {IsStreamingRole, TimestampRole});
}

void MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
    emit countChanged();
}

QString MessageModel::lastAssistantMessage() const {
    for (int i = m_messages.count() - 1; i >= 0; --i) {
        if (m_messages[i].role == "assistant")
            return m_messages[i].content;
    }
    return {};
}

QList<ChatMessage> MessageModel::toChatMessages() const {
    QList<ChatMessage> result;
    for (const auto &msg : m_messages) {
        if (msg.isError) continue;
        result.append({msg.role, msg.content, msg.thinkingContent, msg.attachments});
    }
    return result;
}

QJsonArray MessageModel::toJsonArray() const {
    QJsonArray arr;
    for (const auto &msg : m_messages) {
        QJsonObject obj;
        obj["role"] = msg.role;
        obj["content"] = msg.content;
        obj["thinking"] = msg.thinkingContent;
        obj["timestamp"] = msg.timestamp.toString(Qt::ISODate);
        obj["isError"] = msg.isError;
        if (!msg.attachments.isEmpty()) {
            QJsonArray attachArr;
            for (const auto &a : msg.attachments)
                attachArr.append(a);
            obj["attachments"] = attachArr;
        }
        arr.append(obj);
    }
    return arr;
}

void MessageModel::fromJsonArray(const QJsonArray &arr) {
    beginResetModel();
    m_messages.clear();
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        Message msg;
        msg.role = obj["role"].toString();
        msg.content = obj["content"].toString();
        msg.thinkingContent = obj["thinking"].toString();
        msg.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        msg.isError = obj["isError"].toBool();
        if (obj.contains("attachments")) {
            for (const auto &a : obj["attachments"].toArray())
                msg.attachments.append(a.toString());
        }
        m_messages.append(msg);
    }
    endResetModel();
    emit countChanged();
}
