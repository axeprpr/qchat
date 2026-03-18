#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QUuid>

class ConversationListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        LastMessageRole,
        TimestampRole,
        ModelRole,
        MessageCountRole,
    };

    struct Conversation {
        QString id;
        QString title;
        QString lastMessage;
        QDateTime timestamp;
        QString model;
        int messageCount = 0;
        QJsonArray messages;
        // Per-conversation settings
        QString provider;
        QString systemPrompt;
        double temperature = -1.0; // -1 means use global
        QString parameters;
        bool markdownEnabled = true;
        bool historyToolEnabled = true;
    };

    explicit ConversationListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int idx);

    Q_INVOKABLE QString createConversation(const QString &title = "New Chat");
    Q_INVOKABLE void deleteConversation(int index);
    Q_INVOKABLE void renameConversation(int index, const QString &title);
    Q_INVOKABLE void updateConversation(const QString &id, const QJsonArray &messages,
                                         const QString &lastMsg, const QString &model);
    Q_INVOKABLE QJsonObject getConversationSettings(int index) const;
    Q_INVOKABLE void updateConversationSettings(int index, const QJsonObject &settings);

    QJsonArray getMessages(const QString &id) const;
    void saveToFile(const QString &path);
    void loadFromFile(const QString &path);

signals:
    void countChanged();
    void currentIndexChanged();
    void conversationSelected(const QString &id, const QJsonArray &messages);

private:
    QList<Conversation> m_conversations;
    int m_currentIndex = -1;
};
