#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QImage>

class ImageHelper : public QObject {
    Q_OBJECT
public:
    explicit ImageHelper(QObject *parent = nullptr);

    // Convert image file to base64 data URL
    Q_INVOKABLE QString imageToBase64(const QString &filePath);

    // Get MIME type from file extension
    Q_INVOKABLE QString getMimeType(const QString &filePath);

    // Check if file is an image
    Q_INVOKABLE bool isImageFile(const QString &filePath);

    // Resize image if too large (max 2048px on longest side)
    Q_INVOKABLE QByteArray resizeAndEncode(const QString &filePath, int maxSize = 2048);

private:
    QString getBase64FromImage(const QImage &image, const QString &format);
};
