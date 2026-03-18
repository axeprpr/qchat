#include "ImageHelper.h"
#include <QFile>
#include <QBuffer>
#include <QImageReader>
#include <QFileInfo>

ImageHelper::ImageHelper(QObject *parent) : QObject(parent) {}

QString ImageHelper::imageToBase64(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QByteArray data = file.readAll();
    QString mimeType = getMimeType(filePath);
    QString base64 = data.toBase64();

    return QString("data:%1;base64,%2").arg(mimeType, base64);
}

QString ImageHelper::getMimeType(const QString &filePath) {
    QString ext = QFileInfo(filePath).suffix().toLower();

    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "webp") return "image/webp";
    if (ext == "bmp") return "image/bmp";

    return "image/jpeg"; // default
}

bool ImageHelper::isImageFile(const QString &filePath) {
    QStringList imageExts = {"png", "jpg", "jpeg", "gif", "webp", "bmp"};
    QString ext = QFileInfo(filePath).suffix().toLower();
    return imageExts.contains(ext);
}

QByteArray ImageHelper::resizeAndEncode(const QString &filePath, int maxSize) {
    QImageReader reader(filePath);
    QImage image = reader.read();

    if (image.isNull()) {
        return QByteArray();
    }

    // Resize if needed
    if (image.width() > maxSize || image.height() > maxSize) {
        image = image.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Encode to JPEG with quality 85
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG", 85);

    return ba;
}

QString ImageHelper::getBase64FromImage(const QImage &image, const QString &format) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toUtf8().constData());

    return ba.toBase64();
}
