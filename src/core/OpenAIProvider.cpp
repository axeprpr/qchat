#include "OpenAIProvider.h"
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>

OpenAIProvider::OpenAIProvider(QObject *parent) : ModelProvider(parent) {
    m_networkManager = new QNetworkAccessManager(this);
}

QString OpenAIProvider::providerName() const {
    return m_config.name.isEmpty() ? "OpenAI" : m_config.name;
}

QStringList OpenAIProvider::defaultModels() const {
    return {
        "gpt-4o", "gpt-4o-mini", "gpt-4-turbo",
        "o1", "o1-mini", "o3-mini",
        "deepseek-chat", "deepseek-reasoner"
    };
}

bool OpenAIProvider::supportsThinking() const {
    return true; // o1/o3/deepseek-reasoner models support reasoning
}

bool OpenAIProvider::supportsVision() const {
    return true; // gpt-4o, gpt-4-turbo support vision
}

void OpenAIProvider::sendMessage(const QList<ChatMessage> &messages,
                                  const QString &model,
                                  double temperature,
                                  int maxTokens,
                                  bool enableThinking) {
    cancelRequest();
    m_fullResponse.clear();
    m_thinkingContent.clear();
    m_inThinking = false;
    m_buffer.clear();

    QUrl url(m_config.baseUrl.isEmpty() ? "https://api.openai.com/v1/chat/completions"
                                        : m_config.baseUrl + "/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_config.apiKey).toUtf8());

    QJsonObject body = buildRequestBody(messages, model, temperature, maxTokens, enableThinking);

    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(m_currentReply, &QNetworkReply::readyRead, this, &OpenAIProvider::onStreamData);
    connect(m_currentReply, &QNetworkReply::finished, this, &OpenAIProvider::onReplyFinished);
}

void OpenAIProvider::cancelRequest() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

QJsonObject OpenAIProvider::buildRequestBody(const QList<ChatMessage> &messages,
                                              const QString &model,
                                              double temperature,
                                              int maxTokens,
                                              bool enableThinking) {
    QJsonObject body;
    body["model"] = model.isEmpty() ? m_config.defaultModel : model;
    body["messages"] = messagesToJson(messages);
    body["stream"] = true;

    // For reasoning models (o1, o3, deepseek-reasoner), handle differently
    bool isReasoningModel = model.contains("o1") || model.contains("o3") ||
                            model.contains("reasoner") || model.contains("think");

    if (isReasoningModel && enableThinking) {
        // DeepSeek reasoner or OpenAI o-series
        if (model.contains("deepseek")) {
            // DeepSeek uses stream_options for reasoning_content
            QJsonObject streamOptions;
            streamOptions["include_usage"] = true;
            body["stream_options"] = streamOptions;
        }
        // Don't set temperature for reasoning models
    } else {
        body["temperature"] = temperature;
        if (maxTokens > 0)
            body["max_tokens"] = maxTokens;
    }

    return body;
}

QJsonArray OpenAIProvider::messagesToJson(const QList<ChatMessage> &messages) {
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

            // Add text content
            if (!msg.content.isEmpty()) {
                QJsonObject textPart;
                textPart["type"] = "text";
                textPart["text"] = msg.content;
                contentArray.append(textPart);
            }

            // Add image parts
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
                        QString mimeType = "image/jpeg";
                        if (path.endsWith(".png", Qt::CaseInsensitive)) mimeType = "image/png";
                        else if (path.endsWith(".webp", Qt::CaseInsensitive)) mimeType = "image/webp";
                        else if (path.endsWith(".gif", Qt::CaseInsensitive)) mimeType = "image/gif";

                        QJsonObject imagePart;
                        imagePart["type"] = "image_url";
                        QJsonObject imageUrl;
                        imageUrl["url"] = QString("data:%1;base64,%2").arg(mimeType, base64);
                        imagePart["image_url"] = imageUrl;
                        contentArray.append(imagePart);
                    }
                }
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

void OpenAIProvider::onStreamData() {
    if (!m_currentReply) return;

    m_buffer += m_currentReply->readAll();

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (!line.isEmpty())
            parseSSELine(line);
    }
}

void OpenAIProvider::parseSSELine(const QString &line) {
    if (!line.startsWith("data: ")) return;

    QString data = line.mid(6).trimmed();
    if (data == "[DONE]") return;

    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty()) return;

    QJsonObject delta = choices[0].toObject()["delta"].toObject();

    // Handle reasoning/thinking content (DeepSeek reasoner format)
    if (delta.contains("reasoning_content")) {
        QString thinking = delta["reasoning_content"].toString();
        if (!thinking.isEmpty()) {
            m_thinkingContent += thinking;
            emit thinkingChunk(thinking);
        }
        return;
    }

    // Handle standard content
    if (delta.contains("content")) {
        QString content = delta["content"].toString();
        if (!content.isEmpty()) {
            // Check for <think> tags (some models use XML-style thinking)
            if (content.contains("<think>")) {
                m_inThinking = true;
                content = content.mid(content.indexOf("<think>") + 7);
            }
            if (m_inThinking) {
                int endIdx = content.indexOf("</think>");
                if (endIdx >= 0) {
                    m_thinkingContent += content.left(endIdx);
                    emit thinkingChunk(content.left(endIdx));
                    m_inThinking = false;
                    content = content.mid(endIdx + 8);
                    if (!content.isEmpty()) {
                        m_fullResponse += content;
                        emit responseChunk(content);
                    }
                } else {
                    m_thinkingContent += content;
                    emit thinkingChunk(content);
                }
            } else {
                m_fullResponse += content;
                emit responseChunk(content);
            }
        }
    }
}

void OpenAIProvider::onReplyFinished() {
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
    } else if (m_currentReply->error() == QNetworkReply::NoError) {
        emit responseFinished(m_fullResponse, m_thinkingContent);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void OpenAIProvider::fetchModels() {
    QUrl url(m_config.baseUrl.isEmpty() ? "https://api.openai.com/v1/models"
                                        : m_config.baseUrl + "/models");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_config.apiKey).toUtf8());

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QStringList models;
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray data = doc.object()["data"].toArray();
            for (const auto &item : data) {
                models.append(item.toObject()["id"].toString());
            }
            models.sort();
        }
        emit modelsLoaded(models);
        reply->deleteLater();
    });
}
