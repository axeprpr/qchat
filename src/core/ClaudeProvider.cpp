#include "ClaudeProvider.h"
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>

ClaudeProvider::ClaudeProvider(QObject *parent) : ModelProvider(parent) {
    m_networkManager = new QNetworkAccessManager(this);
}

QString ClaudeProvider::providerName() const {
    return "Anthropic Claude";
}

QStringList ClaudeProvider::defaultModels() const {
    return {
        "claude-opus-4-6",
        "claude-sonnet-4-6",
        "claude-haiku-4-5-20251001",
        "claude-sonnet-4-5-20250514",
    };
}

void ClaudeProvider::sendMessage(const QList<ChatMessage> &messages,
                                  const QString &model,
                                  double temperature,
                                  int maxTokens,
                                  bool enableThinking) {
    cancelRequest();
    m_fullResponse.clear();
    m_thinkingContent.clear();
    m_buffer.clear();
    m_currentEvent.clear();

    QUrl url(m_config.baseUrl.isEmpty() ? "https://api.anthropic.com/v1/messages"
                                        : m_config.baseUrl + "/messages");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-api-key", m_config.apiKey.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");

    QJsonObject body = buildRequestBody(messages, model, temperature, maxTokens, enableThinking);

    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(m_currentReply, &QNetworkReply::readyRead, this, &ClaudeProvider::onStreamData);
    connect(m_currentReply, &QNetworkReply::finished, this, &ClaudeProvider::onReplyFinished);
}

void ClaudeProvider::cancelRequest() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

QJsonObject ClaudeProvider::buildRequestBody(const QList<ChatMessage> &messages,
                                              const QString &model,
                                              double temperature,
                                              int maxTokens,
                                              bool enableThinking) {
    QJsonObject body;
    body["model"] = model.isEmpty() ? m_config.defaultModel : model;
    body["stream"] = true;
    body["max_tokens"] = maxTokens > 0 ? maxTokens : 8192;

    // Extract system message
    QString systemPrompt;
    QList<ChatMessage> nonSystemMessages;
    for (const auto &msg : messages) {
        if (msg.role == "system") {
            systemPrompt = msg.content;
        } else {
            nonSystemMessages.append(msg);
        }
    }

    if (!systemPrompt.isEmpty())
        body["system"] = systemPrompt;

    body["messages"] = messagesToJson(nonSystemMessages);

    // Extended thinking support (Claude Sonnet 4.5+)
    if (enableThinking) {
        QJsonObject thinking;
        thinking["type"] = "enabled";
        thinking["budget_tokens"] = 10000;
        body["thinking"] = thinking;
        // When thinking is enabled, temperature must not be set
    } else {
        body["temperature"] = temperature;
    }

    return body;
}

QJsonArray ClaudeProvider::messagesToJson(const QList<ChatMessage> &messages) {
    QJsonArray arr;
    for (const auto &msg : messages) {
        QJsonObject obj;
        obj["role"] = msg.role;

        // Check if message has image attachments
        bool hasImages = false;
        for (const QString &path : msg.attachments) {
            if (path.endsWith(".png", Qt::CaseInsensitive) ||
                path.endsWith(".jpg", Qt::CaseInsensitive) ||
                path.endsWith(".jpeg", Qt::CaseInsensitive) ||
                path.endsWith(".webp", Qt::CaseInsensitive) ||
                path.endsWith(".gif", Qt::CaseInsensitive)) {
                hasImages = true;
                break;
            }
        }

        if (hasImages && msg.role == "user") {
            // Use content array format for vision
            QJsonArray contentArray;

            // Add image parts first (Claude prefers images before text)
            for (const QString &path : msg.attachments) {
                if (path.endsWith(".png", Qt::CaseInsensitive) ||
                    path.endsWith(".jpg", Qt::CaseInsensitive) ||
                    path.endsWith(".jpeg", Qt::CaseInsensitive) ||
                    path.endsWith(".webp", Qt::CaseInsensitive) ||
                    path.endsWith(".gif", Qt::CaseInsensitive)) {

                    // Read and encode image to base64
                    QFile file(path);
                    if (file.open(QIODevice::ReadOnly)) {
                        QByteArray imageData = file.readAll();
                        QString base64 = imageData.toBase64();
                        QString mediaType = "image/jpeg";
                        if (path.endsWith(".png", Qt::CaseInsensitive)) mediaType = "image/png";
                        else if (path.endsWith(".webp", Qt::CaseInsensitive)) mediaType = "image/webp";
                        else if (path.endsWith(".gif", Qt::CaseInsensitive)) mediaType = "image/gif";

                        QJsonObject imagePart;
                        imagePart["type"] = "image";
                        QJsonObject source;
                        source["type"] = "base64";
                        source["media_type"] = mediaType;
                        source["data"] = base64;
                        imagePart["source"] = source;
                        contentArray.append(imagePart);
                    }
                }
            }

            // Add text content after images
            if (!msg.content.isEmpty()) {
                QJsonObject textPart;
                textPart["type"] = "text";
                textPart["text"] = msg.content;
                contentArray.append(textPart);
            }

            obj["content"] = contentArray;
        } else {
            // Plain text content
            obj["content"] = msg.content;
        }

        arr.append(obj);
    }
    return arr;
}

void ClaudeProvider::onStreamData() {
    if (!m_currentReply) return;

    m_buffer += m_currentReply->readAll();

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (line.startsWith("event: ")) {
            m_currentEvent = line.mid(7).trimmed();
        } else if (line.startsWith("data: ")) {
            parseSSEEvent(m_currentEvent, line.mid(6).trimmed());
        }
    }
}

void ClaudeProvider::parseSSEEvent(const QString &event, const QString &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();

    if (event == "content_block_delta") {
        QJsonObject delta = obj["delta"].toObject();
        QString type = delta["type"].toString();

        if (type == "thinking_delta") {
            QString thinking = delta["thinking"].toString();
            if (!thinking.isEmpty()) {
                m_thinkingContent += thinking;
                emit thinkingChunk(thinking);
            }
        } else if (type == "text_delta") {
            QString text = delta["text"].toString();
            if (!text.isEmpty()) {
                m_fullResponse += text;
                emit responseChunk(text);
            }
        }
    } else if (event == "message_stop") {
        emit responseFinished(m_fullResponse, m_thinkingContent);
    } else if (event == "error") {
        QJsonObject error = obj["error"].toObject();
        emit errorOccurred(error["message"].toString());
    }
}

void ClaudeProvider::onReplyFinished() {
    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError &&
        m_currentReply->error() != QNetworkReply::OperationCanceledError) {
        QByteArray errorData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(errorData);
        QString errorMsg;
        if (doc.isObject() && doc.object().contains("error")) {
            errorMsg = doc.object()["error"].toObject()["message"].toString();
        } else {
            errorMsg = m_currentReply->errorString();
        }
        emit errorOccurred(errorMsg);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}
