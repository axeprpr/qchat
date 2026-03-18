#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QDateTime>
#include "ModelProvider.h"

class MessageModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        RoleRole = Qt::UserRole + 1,
        ContentRole,
        ThinkingRole,
        TimestampRole,
        IsStreamingRole,
        AttachmentsRole,
        IsErrorRole,
    };

    struct Message {
        QString role;
        QString content;
        QString thinkingContent;
        QDateTime timestamp;
        bool isStreaming = false;
        QStringList attachments;
        bool isError = false;
    };

    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addMessage(const QString &role, const QString &content,
                                 const QStringList &attachments = {});
    Q_INVOKABLE void addErrorMessage(const QString &error);
    Q_INVOKABLE void appendToLastMessage(const QString &text);
    Q_INVOKABLE void appendThinking(const QString &text);
    Q_INVOKABLE void finishStreaming();
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString lastAssistantMessage() const;

    void startAssistantMessage();
    QList<ChatMessage> toChatMessages() const;
    QJsonArray toJsonArray() const;
    void fromJsonArray(const QJsonArray &arr);

signals:
    void countChanged();
    void streamingUpdated();

private:
    QList<Message> m_messages;
};
