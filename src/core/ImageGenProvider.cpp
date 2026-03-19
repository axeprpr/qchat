#include "ImageGenProvider.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

ImageGenProvider::ImageGenProvider(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void ImageGenProvider::generateImage(const QString &prompt,
                                     const QString &size,
                                     const QString &quality,
                                     const QString &style) {
    if (m_isGenerating) return;

    QJsonObject body;
    body["model"] = "dall-e-3";
    body["prompt"] = prompt;
    body["n"] = 1;
    body["size"] = size;
    body["quality"] = quality;
    body["style"] = style;
    body["response_format"] = "url";

    QNetworkRequest req(QUrl(m_baseUrl + "/images/generations"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    m_isGenerating = true;
    emit isGeneratingChanged();

    m_reply = m_nam->post(req, QJsonDocument(body).toJson());
    connect(m_reply, &QNetworkReply::finished, this, &ImageGenProvider::onReplyFinished);
}

void ImageGenProvider::cancel() {
    if (m_reply) {
        m_reply->abort();
        m_reply = nullptr;
    }
    m_isGenerating = false;
    emit isGeneratingChanged();
}

void ImageGenProvider::onReplyFinished() {
    m_isGenerating = false;
    emit isGeneratingChanged();

    if (!m_reply) return;
    auto *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QByteArray body = reply->readAll();
        QJsonObject errObj = QJsonDocument::fromJson(body).object();
        QString msg = errObj["error"].toObject()["message"].toString();
        if (msg.isEmpty()) msg = reply->errorString();
        emit errorOccurred(msg);
        return;
    }

    QJsonObject resp = QJsonDocument::fromJson(reply->readAll()).object();
    QJsonArray data = resp["data"].toArray();
    if (data.isEmpty()) {
        emit errorOccurred("No image returned");
        return;
    }

    QJsonObject first = data[0].toObject();
    QString url = first["url"].toString();
    QString revised = first["revised_prompt"].toString();
    emit imageReady(url, revised);
}
