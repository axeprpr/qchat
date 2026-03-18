#include "VoiceManager.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>

VoiceManager::VoiceManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{}

VoiceManager::~VoiceManager() {
    stopSpeaking();
}

QStringList VoiceManager::availableVoices() const {
    return {"alloy", "echo", "fable", "onyx", "nova", "shimmer"};
}

void VoiceManager::setApiKey(const QString &key) {
    m_apiKey = key;
}

void VoiceManager::startRecording() {
    if (m_isListening) return;
    m_recordedAudio.clear();
    m_isListening = true;
    emit isListeningChanged();
    // Qt Multimedia recording would be wired here via QMediaCaptureSession
    // For now emit a placeholder — full impl requires Qt6::Multimedia
}

void VoiceManager::stopRecording() {
    if (!m_isListening) return;
    m_isListening = false;
    emit isListeningChanged();
    if (!m_recordedAudio.isEmpty()) {
        sendToWhisper(m_recordedAudio);
    }
}

void VoiceManager::sendToWhisper(const QByteArray &audioData) {
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not set for voice transcription");
        return;
    }

    QNetworkRequest request(QUrl("https://api.openai.com/v1/audio/transcriptions"));
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"file\"; filename=\"audio.wav\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav"));
    filePart.setBody(audioData);

    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"model\""));
    modelPart.setBody("whisper-1");

    multiPart->append(filePart);
    multiPart->append(modelPart);

    m_currentReply = m_network->post(request, multiPart);
    multiPart->setParent(m_currentReply);
    connect(m_currentReply, &QNetworkReply::finished, this, &VoiceManager::onTranscriptionReply);
}

void VoiceManager::onTranscriptionReply() {
    if (!m_currentReply) return;
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(m_currentReply->readAll());
        QString text = doc.object()["text"].toString();
        emit transcriptionReady(text);
    } else {
        emit errorOccurred("Transcription error: " + m_currentReply->errorString());
    }
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void VoiceManager::speak(const QString &text, const QString &voice) {
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not set for TTS");
        return;
    }
    if (m_isSpeaking) stopSpeaking();

    QNetworkRequest request(QUrl("https://api.openai.com/v1/audio/speech"));
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["model"] = "tts-1";
    body["input"] = text;
    body["voice"] = voice;

    m_isSpeaking = true;
    emit isSpeakingChanged();

    m_currentReply = m_network->post(request, QJsonDocument(body).toJson());
    connect(m_currentReply, &QNetworkReply::finished, this, &VoiceManager::onTtsReply);
}

void VoiceManager::onTtsReply() {
    if (!m_currentReply) return;
    if (m_currentReply->error() != QNetworkReply::NoError) {
        emit errorOccurred("TTS error: " + m_currentReply->errorString());
    }
    // Audio playback via QMediaPlayer would be wired here
    m_isSpeaking = false;
    emit isSpeakingChanged();
    emit speechFinished();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void VoiceManager::stopSpeaking() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    if (m_isSpeaking) {
        m_isSpeaking = false;
        emit isSpeakingChanged();
        emit speechFinished();
    }
}
