#include "OllamaProvider.h"
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUrl>

OllamaProvider::OllamaProvider(QObject *parent) : ModelProvider(parent) {
    m_networkManager = new QNetworkAccessManager(this);
}

QString OllamaProvider::providerName() const {
    return "Ollama (Local)";
}

QStringList OllamaProvider::defaultModels() const {
    return {
        "llama3.3", "qwen3.5", "deepseek-r1", "deepseek-v3",
        "gemma3", "phi4", "mistral", "codellama"
    };
}

void OllamaProvider::sendMessage(const QList<ChatMessage> &messages,
                                  const QString &model,
                                  double temperature,
                                  int maxTokens,
                                  bool enableThinking) {
    Q_UNUSED(enableThinking)
    cancelRequest();
    m_fullResponse.clear();
    m_thinkingContent.clear();
    m_inThinking = false;
    m_buffer.clear();

    QString baseUrl = m_config.baseUrl.isEmpty() ? "http://localhost:11434" : m_config.baseUrl;
    QUrl url(baseUrl + "/api/chat");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["model"] = model.isEmpty() ? m_config.defaultModel : model;
    body["stream"] = true;

    QJsonObject options;
    options["temperature"] = temperature;
    if (maxTokens > 0) options["num_predict"] = maxTokens;
    body["options"] = options;

    QJsonArray msgs;
    for (const auto &msg : messages) {
        QJsonObject m;
        m["role"] = msg.role;
        m["content"] = msg.content;
        msgs.append(m);
    }
    body["messages"] = msgs;

    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(m_currentReply, &QNetworkReply::readyRead, this, &OllamaProvider::onStreamData);
    connect(m_currentReply, &QNetworkReply::finished, this, &OllamaProvider::onReplyFinished);
}

void OllamaProvider::cancelRequest() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void OllamaProvider::onStreamData() {
    if (!m_currentReply) return;

    m_buffer += m_currentReply->readAll();

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();
        if (obj["done"].toBool()) continue;

        QJsonObject message = obj["message"].toObject();
        QString content = message["content"].toString();

        if (content.isEmpty()) continue;

        // Handle <think> blocks from reasoning models (DeepSeek-R1, etc.)
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

void OllamaProvider::onReplyFinished() {
    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError &&
        m_currentReply->error() != QNetworkReply::OperationCanceledError) {
        emit errorOccurred(m_currentReply->errorString());
    } else if (m_currentReply->error() == QNetworkReply::NoError) {
        emit responseFinished(m_fullResponse, m_thinkingContent);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void OllamaProvider::fetchModels() {
    QString baseUrl = m_config.baseUrl.isEmpty() ? "http://localhost:11434" : m_config.baseUrl;
    QUrl url(baseUrl + "/api/tags");

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QStringList models;
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray data = doc.object()["models"].toArray();
            for (const auto &item : data) {
                models.append(item.toObject()["name"].toString());
            }
        }
        emit modelsLoaded(models);
        reply->deleteLater();
    });
}
