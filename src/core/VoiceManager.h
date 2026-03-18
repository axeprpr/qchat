#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QByteArray>

class VoiceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isListening READ isListening NOTIFY isListeningChanged)
    Q_PROPERTY(bool isSpeaking READ isSpeaking NOTIFY isSpeakingChanged)
    Q_PROPERTY(QStringList availableVoices READ availableVoices CONSTANT)

public:
    explicit VoiceManager(QObject *parent = nullptr);
    ~VoiceManager();

    bool isListening() const { return m_isListening; }
    bool isSpeaking() const { return m_isSpeaking; }
    QStringList availableVoices() const;

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void speak(const QString &text, const QString &voice = "alloy");
    Q_INVOKABLE void stopSpeaking();
    Q_INVOKABLE void setApiKey(const QString &key);

signals:
    void isListeningChanged();
    void isSpeakingChanged();
    void transcriptionReady(const QString &text);
    void speechFinished();
    void errorOccurred(const QString &error);

private slots:
    void onTranscriptionReply();
    void onTtsReply();

private:
    void sendToWhisper(const QByteArray &audioData);

    QNetworkAccessManager *m_network;
    QNetworkReply *m_currentReply = nullptr;
    bool m_isListening = false;
    bool m_isSpeaking = false;
    QString m_apiKey;
    QByteArray m_recordedAudio;
};
