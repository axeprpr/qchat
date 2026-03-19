#include "ExportHelper.h"
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QDateTime>
#include <QPrinter>
#include <QTextDocument>

ExportHelper::ExportHelper(QObject *parent) : QObject(parent) {}

bool ExportHelper::exportConversation(const QJsonArray &messages,
                                       const QString &filePath,
                                       ExportFormat format) {
    switch (format) {
    case Markdown: {
        QString content = toMarkdown(messages);
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << content;
        return true;
    }
    case HTML: {
        QString content = toHTML(messages);
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << content;
        return true;
    }
    case PDF:
        return toPDF(messages, filePath);
    }
    return false;
}

QString ExportHelper::toMarkdown(const QJsonArray &messages) {
    QString md;
    md += "# QChat Conversation Export\n\n";
    md += QString("**Exported:** %1\n\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    md += "---\n\n";

    for (const auto &val : messages) {
        QJsonObject msg = val.toObject();
        QString role = msg["role"].toString();
        QString content = msg["content"].toString();
        QString thinking = msg["thinkingContent"].toString();

        md += QString("## %1\n\n").arg(role == "user" ? "You" : "Assistant");

        if (!thinking.isEmpty()) {
            md += "> **Thinking:**\n";
            for (const QString &line : thinking.split('\n')) {
                md += "> " + line + "\n";
            }
            md += "\n";
        }

        md += content + "\n\n";
        md += "---\n\n";
    }

    return md;
}

QString ExportHelper::toHTML(const QJsonArray &messages) {
    QString html = getHTMLTemplate();
    QString messagesHtml;

    for (const auto &val : messages) {
        QJsonObject msg = val.toObject();
        QString role = msg["role"].toString();
        QString content = msg["content"].toString();
        QString thinking = msg["thinkingContent"].toString();
        bool isUser = (role == "user");

        messagesHtml += QString("<div class='message %1'>\n").arg(isUser ? "user" : "assistant");
        messagesHtml += QString("<div class='role'>%1</div>\n").arg(isUser ? "You" : "Assistant");

        if (!thinking.isEmpty()) {
            messagesHtml += "<div class='thinking'>\n";
            messagesHtml += "<strong>Thinking:</strong><br>\n";
            messagesHtml += "<pre>" + escapeHtml(thinking) + "</pre>\n";
            messagesHtml += "</div>\n";
        }

        messagesHtml += "<div class='content'>\n";
        messagesHtml += escapeHtml(content).replace("\n", "<br>\n");
        messagesHtml += "</div>\n";
        messagesHtml += "</div>\n\n";
    }

    html.replace("{{MESSAGES}}", messagesHtml);
    html.replace("{{DATE}}", QDateTime::currentDateTime().toString(Qt::ISODate));
    return html;
}

bool ExportHelper::toPDF(const QJsonArray &messages, const QString &filePath) {
    QString htmlContent = toHTML(messages);

    QTextDocument doc;
    doc.setHtml(htmlContent);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    doc.print(&printer);
    return true;
}

QString ExportHelper::escapeHtml(const QString &text) {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

QString ExportHelper::getHTMLTemplate() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>QChat Conversation Export</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            max-width: 800px;
            margin: 40px auto;
            padding: 20px;
            background: #f5f5f5;
            color: #1a1a1a;
        }
        h1 {
            color: #4361ee;
            border-bottom: 2px solid #4361ee;
            padding-bottom: 10px;
        }
        .export-info {
            color: #666;
            font-size: 14px;
            margin-bottom: 30px;
        }
        .message {
            background: white;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 16px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .message.user {
            background: #e3f2fd;
        }
        .role {
            font-weight: bold;
            font-size: 12px;
            text-transform: uppercase;
            color: #666;
            margin-bottom: 8px;
        }
        .thinking {
            background: #f0f4ff;
            border-left: 3px solid #4361ee;
            padding: 12px;
            margin-bottom: 12px;
            font-size: 13px;
        }
        .thinking pre {
            margin: 8px 0 0 0;
            white-space: pre-wrap;
            font-family: 'Courier New', monospace;
            font-size: 12px;
        }
        .content {
            line-height: 1.6;
            white-space: pre-wrap;
        }
    </style>
</head>
<body>
    <h1>QChat Conversation Export</h1>
    <div class="export-info">Exported: {{DATE}}</div>
    {{MESSAGES}}
</body>
</html>)";
}
