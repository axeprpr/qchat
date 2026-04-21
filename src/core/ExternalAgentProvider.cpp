#include "ExternalAgentProvider.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

ExternalAgentProvider::ExternalAgentProvider(const QString &name, QObject *parent)
    : ModelProvider(parent)
    , m_name(name)
{
    m_networkManager = new QNetworkAccessManager(this);
}

QString ExternalAgentProvider::providerName() const {
    return m_name;
}

QStringList ExternalAgentProvider::defaultModels() const {
    return {};
}

void ExternalAgentProvider::setConversationContext(const QJsonObject &context) {
    m_context = context;
}

QString ExternalAgentProvider::normalizeBaseUrl() const {
    QString base = m_config.baseUrl.trimmed();
    while (base.endsWith('/')) {
        base.chop(1);
    }
    return base;
}

QString ExternalAgentProvider::extractTextContent(const QJsonValue &contentVal) const {
    if (contentVal.isString()) {
        return contentVal.toString();
    }

    if (contentVal.isArray()) {
        QString text;
        const QJsonArray arr = contentVal.toArray();
        for (const auto &item : arr) {
            if (item.isString()) {
                text += item.toString();
                continue;
            }
            if (!item.isObject()) {
                continue;
            }
            const QJsonObject obj = item.toObject();
            const QString type = obj.value("type").toString();
            if (type == "text") {
                text += obj.value("text").toString();
            } else if (obj.value("text").isString()) {
                text += obj.value("text").toString();
            } else if (obj.value("content").isString()) {
                text += obj.value("content").toString();
            }
        }
        return text;
    }

    if (contentVal.isObject()) {
        const QJsonObject obj = contentVal.toObject();
        if (obj.value("text").isString()) {
            return obj.value("text").toString();
        }
        if (obj.value("content").isString()) {
            return obj.value("content").toString();
        }
    }

    return {};
}

void ExternalAgentProvider::sendMessage(const QList<ChatMessage> &messages,
                                        const QString &model,
                                        double temperature,
                                        int maxTokens,
                                        bool enableThinking) {
    Q_UNUSED(model)
    Q_UNUSED(temperature)
    Q_UNUSED(maxTokens)
    Q_UNUSED(enableThinking)

    cancelRequest();
    m_fullResponse.clear();
    m_thinkingContent.clear();
    m_buffer.clear();
    m_sseEvent.clear();

    m_threadId = m_context.value("conversation_id").toString().trimmed();
    m_runId = m_context.value("run_id").toString().trimmed();

    QString query;
    for (int i = messages.count() - 1; i >= 0; --i) {
        if (messages[i].role == "user") {
            query = messages[i].content;
            break;
        }
    }

    if (query.trimmed().isEmpty()) {
        emit errorOccurred("Empty user query");
        return;
    }

    bool ok = false;
    if (m_name == "Dify") {
        ok = sendDifyMessage(query);
    } else if (m_name == "DeerFlow") {
        ok = sendDeerFlowMessage(query);
    } else {
        emit errorOccurred("Unsupported external provider: " + m_name);
        return;
    }

    if (!ok) {
        return;
    }
}

bool ExternalAgentProvider::sendDifyMessage(const QString &query) {
    const QString base = normalizeBaseUrl();
    if (base.isEmpty()) {
        emit errorOccurred("Base URL not configured for Dify");
        return false;
    }

    const QString apiKey = m_config.apiKey.trimmed();
    if (apiKey.isEmpty()) {
        emit errorOccurred("Dify API key is required");
        return false;
    }

    QJsonObject body;
    QJsonObject inputs;
    if (m_context.value("qchat_skills").isArray()) {
        inputs["qchat_skills"] = m_context.value("qchat_skills").toArray();
    } else if (m_config.extra.value("qchat_skills").isArray()) {
        inputs["qchat_skills"] = m_config.extra.value("qchat_skills").toArray();
    }
    if (m_context.value("qchat_mcp_servers").isArray()) {
        inputs["qchat_mcp_servers"] = m_context.value("qchat_mcp_servers").toArray();
    } else if (m_config.extra.value("qchat_mcp_servers").isArray()) {
        inputs["qchat_mcp_servers"] = m_config.extra.value("qchat_mcp_servers").toArray();
    }
    body["inputs"] = inputs;
    body["query"] = query;
    body["files"] = QJsonArray{};
    body["response_mode"] = "streaming";

    const QString conversationId = m_context.value("conversation_id").toString().trimmed();
    const QString parentMessageId = m_context.value("last_message_id").toString().trimmed();
    if (!conversationId.isEmpty()) {
        body["conversation_id"] = conversationId;
    }
    if (!parentMessageId.isEmpty()) {
        body["parent_message_id"] = parentMessageId;
    }

    const QString userId = m_config.extra.value("userId").toString().trimmed();
    body["user"] = userId.isEmpty() ? QString("qchat-user") : userId;

    QNetworkRequest request(QUrl(base + "/chat-messages"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "text/event-stream");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    m_currentReply = m_networkManager->post(
        request,
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(m_currentReply, &QNetworkReply::readyRead, this, &ExternalAgentProvider::onStreamData);
    connect(m_currentReply, &QNetworkReply::finished, this, &ExternalAgentProvider::onReplyFinished);
    return true;
}

QString ExternalAgentProvider::ensureDeerFlowThreadId() {
    if (!m_threadId.isEmpty()) {
        return m_threadId;
    }

    const QString base = normalizeBaseUrl();
    if (base.isEmpty()) {
        emit errorOccurred("Base URL not configured for DeerFlow");
        return {};
    }

    QNetworkRequest request(QUrl(base + "/langgraph/threads"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QString token = m_config.extra.value("authToken").toString().trimmed().isEmpty()
        ? m_config.apiKey.trimmed()
        : m_config.extra.value("authToken").toString().trimmed();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->post(request, QByteArray("{}"));
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(15000);
    loop.exec();

    if (!timer.isActive()) {
        reply->abort();
        reply->deleteLater();
        emit errorOccurred("DeerFlow create thread timeout");
        return {};
    }

    const QByteArray raw = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        const QString err = reply->errorString();
        reply->deleteLater();
        emit errorOccurred("DeerFlow create thread failed: " + err);
        return {};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    reply->deleteLater();
    if (!doc.isObject()) {
        emit errorOccurred("DeerFlow create thread invalid response");
        return {};
    }

    m_threadId = doc.object().value("thread_id").toString().trimmed();
    if (m_threadId.isEmpty()) {
        emit errorOccurred("DeerFlow thread_id missing in response");
        return {};
    }

    emitSessionIfReady();
    return m_threadId;
}

bool ExternalAgentProvider::sendDeerFlowMessage(const QString &query) {
    const QString base = normalizeBaseUrl();
    if (base.isEmpty()) {
        emit errorOccurred("Base URL not configured for DeerFlow");
        return false;
    }

    const QString assistantId = m_config.extra.value("assistantId").toString().trimmed();
    if (assistantId.isEmpty()) {
        emit errorOccurred("DeerFlow assistantId is required");
        return false;
    }

    const QString threadId = ensureDeerFlowThreadId();
    if (threadId.isEmpty()) {
        return false;
    }

    const QString mode = m_config.extra.value("mode").toString().trimmed().isEmpty()
        ? QString("flash")
        : m_config.extra.value("mode").toString().trimmed();
    const QString modelName = m_config.extra.value("modelName").toString().trimmed();
    const QString agentName = m_config.extra.value("agentName").toString().trimmed();

    QJsonObject message;
    message["type"] = "human";
    message["content"] = query;
    message["additional_kwargs"] = QJsonObject{{"files", QJsonArray{}}};

    QJsonObject input;
    input["messages"] = QJsonArray{message};

    QJsonObject context;
    context["mode"] = mode;
    context["thread_id"] = threadId;
    if (m_context.value("qchat_skills").isArray()) {
        context["qchat_skills"] = m_context.value("qchat_skills").toArray();
    } else if (m_config.extra.value("qchat_skills").isArray()) {
        context["qchat_skills"] = m_config.extra.value("qchat_skills").toArray();
    }
    if (m_context.value("qchat_mcp_servers").isArray()) {
        context["qchat_mcp_servers"] = m_context.value("qchat_mcp_servers").toArray();
    } else if (m_config.extra.value("qchat_mcp_servers").isArray()) {
        context["qchat_mcp_servers"] = m_config.extra.value("qchat_mcp_servers").toArray();
    }
    if (!modelName.isEmpty()) {
        context["model_name"] = modelName;
    }
    if (!agentName.isEmpty()) {
        context["agent_name"] = agentName;
    }

    QJsonObject body;
    body["assistant_id"] = assistantId;
    body["input"] = input;
    body["config"] = QJsonObject{{"recursion_limit", 1000}};
    body["context"] = context;
    body["stream_mode"] = QJsonArray{"messages", "custom"};
    body["on_disconnect"] = "cancel";

    QNetworkRequest request(QUrl(base + "/langgraph/threads/" + threadId + "/runs/stream"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "text/event-stream");

    const QString token = m_config.extra.value("authToken").toString().trimmed().isEmpty()
        ? m_config.apiKey.trimmed()
        : m_config.extra.value("authToken").toString().trimmed();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    m_currentReply = m_networkManager->post(
        request,
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(m_currentReply, &QNetworkReply::readyRead, this, &ExternalAgentProvider::onStreamData);
    connect(m_currentReply, &QNetworkReply::finished, this, &ExternalAgentProvider::onReplyFinished);
    return true;
}

void ExternalAgentProvider::emitSessionIfReady(const QString &messageId) {
    if (m_threadId.isEmpty()) {
        return;
    }

    QJsonObject session;
    session["conversation_id"] = m_threadId;

    if (!messageId.trimmed().isEmpty()) {
        session["last_message_id"] = messageId.trimmed();
    } else {
        const QString lastMessageId = m_context.value("last_message_id").toString().trimmed();
        if (!lastMessageId.isEmpty()) {
            session["last_message_id"] = lastMessageId;
        }
    }

    if (!m_runId.trimmed().isEmpty()) {
        session["run_id"] = m_runId.trimmed();
    }

    emit sessionUpdated(session);
}

void ExternalAgentProvider::cancelRequest() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void ExternalAgentProvider::onStreamData() {
    if (!m_currentReply) {
        return;
    }

    m_buffer += m_currentReply->readAll();
    while (true) {
        const int idx = m_buffer.indexOf('\n');
        if (idx < 0) {
            break;
        }

        QByteArray rawLine = m_buffer.left(idx);
        m_buffer = m_buffer.mid(idx + 1);

        if (!rawLine.isEmpty() && rawLine.endsWith('\r')) {
            rawLine.chop(1);
        }

        const QString line = QString::fromUtf8(rawLine);
        if (!line.isEmpty()) {
            parseSSELine(line);
        }
    }
}

void ExternalAgentProvider::parseSSELine(const QString &line) {
    if (line.startsWith("event:")) {
        m_sseEvent = line.mid(6).trimmed();
        return;
    }

    if (!line.startsWith("data:")) {
        return;
    }

    const QString data = line.mid(5).trimmed();
    if (data.isEmpty() || data == "[DONE]") {
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return;
    }

    if (m_name == "Dify") {
        if (doc.isObject()) {
            handleDifyEvent(doc.object());
        }
        return;
    }

    QJsonValue dataVal;
    if (doc.isObject()) {
        dataVal = doc.object();
    } else if (doc.isArray()) {
        dataVal = doc.array();
    }
    handleDeerFlowEvent(m_sseEvent, dataVal);
}

void ExternalAgentProvider::handleDifyEvent(const QJsonObject &obj) {
    const QString event = obj.value("event").toString();
    const QString conversationId = obj.value("conversation_id").toString().trimmed();
    const QString messageId = obj.value("message_id").toString().trimmed();

    if (!conversationId.isEmpty()) {
        m_threadId = conversationId;
    }

    if (event == "workflow_started") {
        const QString runId = obj.value("workflow_run_id").toString().trimmed();
        if (!runId.isEmpty()) {
            m_runId = runId;
        }
        emitSessionIfReady(messageId);
        return;
    }

    if (event == "node_started") {
        const QJsonObject dataObj = obj.value("data").toObject();
        const QString title = dataObj.value("title").toString();
        const QString nodeId = dataObj.value("node_id").toString();
        const QString line = QString("\n[Node Start] %1 (%2)\n")
            .arg(title.isEmpty() ? QString("node") : title,
                 nodeId.isEmpty() ? QString("unknown") : nodeId);
        m_thinkingContent += line;
        emit thinkingChunk(line);
        emitSessionIfReady(messageId);
        return;
    }

    if (event == "node_finished") {
        const QJsonObject dataObj = obj.value("data").toObject();
        const QString title = dataObj.value("title").toString();
        const QString status = dataObj.value("status").toString();
        QString line = QString("\n[Node Finished] %1 (%2)\n")
            .arg(title.isEmpty() ? QString("node") : title,
                 status.isEmpty() ? QString("unknown") : status);
        if (dataObj.contains("outputs") && dataObj.value("outputs").isObject()) {
            line += QString::fromUtf8(
                QJsonDocument(dataObj.value("outputs").toObject())
                    .toJson(QJsonDocument::Compact));
            line += "\n";
        }
        m_thinkingContent += line;
        emit thinkingChunk(line);
        emitSessionIfReady(messageId);
        return;
    }

    if (event == "message" || event == "agent_message") {
        const QString answer = obj.value("answer").toString();
        if (!answer.isEmpty()) {
            m_fullResponse += answer;
            emit responseChunk(answer);
        }
        emitSessionIfReady(messageId);
        return;
    }

    if (event == "workflow_finished") {
        const QString runId = obj.value("workflow_run_id").toString().trimmed();
        if (!runId.isEmpty()) {
            m_runId = runId;
        }
        emitSessionIfReady(messageId);
        return;
    }

    if (event == "error") {
        QString msg = obj.value("message").toString();
        if (msg.isEmpty() && obj.value("data").isObject()) {
            msg = obj.value("data").toObject().value("message").toString();
        }
        emit errorOccurred(msg.isEmpty() ? QString("Dify stream error") : msg);
        return;
    }

    if (!messageId.isEmpty()) {
        emitSessionIfReady(messageId);
    }
}

void ExternalAgentProvider::handleDeerFlowEvent(const QString &eventName, const QJsonValue &dataVal) {
    if (eventName == "messages") {
        if (!dataVal.isArray()) {
            return;
        }

        const QJsonArray tuple = dataVal.toArray();
        if (tuple.isEmpty() || !tuple.at(0).isObject()) {
            return;
        }

        const QJsonObject messageObj = tuple.at(0).toObject();
        const QJsonObject metadataObj =
            (tuple.size() > 1 && tuple.at(1).isObject()) ? tuple.at(1).toObject() : QJsonObject{};

        const QString runId = metadataObj.value("run_id").toString().trimmed();
        if (!runId.isEmpty()) {
            m_runId = runId;
        }

        const QString messageId = messageObj.value("id").toString().trimmed();
        const QString type = messageObj.value("type").toString();

        if (type == "ai") {
            const QString text = extractTextContent(messageObj.value("content"));
            if (!text.isEmpty()) {
                m_fullResponse += text;
                emit responseChunk(text);
            }

            const QJsonArray toolCalls = messageObj.value("tool_calls").toArray();
            for (const auto &item : toolCalls) {
                if (!item.isObject()) {
                    continue;
                }
                const QJsonObject callObj = item.toObject();
                const QString name = callObj.value("name").toString();
                QString line = QString("\n[Tool] %1\n")
                    .arg(name.isEmpty() ? QString("tool_call") : name);
                if (callObj.value("args").isObject()) {
                    line += QString::fromUtf8(
                        QJsonDocument(callObj.value("args").toObject())
                            .toJson(QJsonDocument::Compact));
                    line += "\n";
                }
                m_thinkingContent += line;
                emit thinkingChunk(line);
            }
        } else if (type == "tool") {
            const QString text = extractTextContent(messageObj.value("content"));
            if (!text.isEmpty()) {
                const QString line = QString("\n[Tool Result]\n%1\n").arg(text);
                m_thinkingContent += line;
                emit thinkingChunk(line);
            }
        }

        emitSessionIfReady(messageId);
        return;
    }

    if (eventName == "custom") {
        if (!dataVal.isObject()) {
            return;
        }

        const QJsonObject obj = dataVal.toObject();
        const QString type = obj.value("type").toString();
        if (type == "task_running" || type == "agent_thought" || type == "status") {
            QString content;
            if (obj.value("message").isObject()) {
                content = extractTextContent(obj.value("message").toObject().value("content"));
            }
            if (content.isEmpty()) {
                content = obj.value("content").toString();
            }
            if (content.isEmpty()) {
                content = obj.value("message").toString();
            }
            if (!content.isEmpty()) {
                const QString line = QString("\n[Task] %1\n").arg(content);
                m_thinkingContent += line;
                emit thinkingChunk(line);
            }
        }
        return;
    }

    if (eventName == "error") {
        if (dataVal.isObject()) {
            const QJsonObject obj = dataVal.toObject();
            QString msg = obj.value("message").toString();
            if (msg.isEmpty()) {
                msg = obj.value("error").toString();
            }
            emit errorOccurred(msg.isEmpty() ? QString("DeerFlow stream error") : msg);
        }
    }
}

void ExternalAgentProvider::onReplyFinished() {
    if (!m_currentReply) {
        return;
    }

    // Flush remaining partial line in buffer.
    if (!m_buffer.isEmpty()) {
        QByteArray rawLine = m_buffer;
        m_buffer.clear();
        if (!rawLine.isEmpty() && rawLine.endsWith('\r')) {
            rawLine.chop(1);
        }
        const QString line = QString::fromUtf8(rawLine);
        if (!line.isEmpty()) {
            parseSSELine(line);
        }
    }

    if (m_currentReply->error() != QNetworkReply::NoError &&
        m_currentReply->error() != QNetworkReply::OperationCanceledError) {
        const QByteArray raw = m_currentReply->readAll();
        QString msg = m_currentReply->errorString();

        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (doc.isObject()) {
            const QJsonObject obj = doc.object();
            if (obj.value("message").isString()) {
                msg = obj.value("message").toString();
            } else if (obj.value("detail").isString()) {
                msg = obj.value("detail").toString();
            }
        }
        emit errorOccurred(msg);
    } else if (m_currentReply->error() == QNetworkReply::NoError) {
        emit responseFinished(m_fullResponse, m_thinkingContent);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}
