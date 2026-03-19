#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class ImageGenProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)

public:
    explicit ImageGenProvider(QObject *parent = nullptr);

    bool isGenerating() const { return m_isGenerating; }

    // Generate image via DALL-E 3
    Q_INVOKABLE void generateImage(const QString &prompt,
                                   const QString &size = "1024x1024",
                                   const QString &quality = "standard",
                                   const QString &style = "vivid");
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void setApiKey(const QString &key) { m_apiKey = key; }
    Q_INVOKABLE void setBaseUrl(const QString &url) { m_baseUrl = url; }

signals:
    void imageReady(const QString &imageUrl, const QString &revisedPrompt);
    void errorOccurred(const QString &error);
    void isGeneratingChanged();

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply = nullptr;
    QString m_apiKey;
    QString m_baseUrl = "https://api.openai.com/v1";
    bool m_isGenerating = false;
};
