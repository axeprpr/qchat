#include "MarkdownHelper.h"
#include <QRegularExpression>

MarkdownHelper::MarkdownHelper(QObject *parent) : QObject(parent) {}

QString MarkdownHelper::escapeHtml(const QString &text) const {
    QString result = text;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    return result;
}

QString MarkdownHelper::processInlineMarkdown(const QString &line) const {
    QString result = line;

    // Bold: **text** or __text__
    result.replace(QRegularExpression(R"(\*\*(.+?)\*\*)"), "<b>\\1</b>");
    result.replace(QRegularExpression(R"(__(.+?)__)"), "<b>\\1</b>");

    // Italic: *text* or _text_
    result.replace(QRegularExpression(R"(\*(.+?)\*)"), "<i>\\1</i>");
    result.replace(QRegularExpression(R"(\b_(.+?)_\b)"), "<i>\\1</i>");

    // Strikethrough: ~~text~~
    result.replace(QRegularExpression(R"(~~(.+?)~~)"), "<s>\\1</s>");

    // Inline code: `text`
    result.replace(QRegularExpression(R"(`([^`]+?)`)"),
                   "<code style='background-color:#2d2d2d;color:#e6e6e6;padding:2px 6px;"
                   "border-radius:4px;font-family:monospace;'>\\1</code>");

    // Links: [text](url)
    result.replace(QRegularExpression(R"(\[(.+?)\]\((.+?)\))"),
                   "<a href='\\2' style='color:#58a6ff;'>\\1</a>");

    return result;
}

QString MarkdownHelper::toHtml(const QString &markdown) const {
    QStringList lines = markdown.split('\n');
    QString html;
    bool inCodeBlock = false;
    QString codeBlockLang;
    QString codeBlockContent;
    bool inList = false;

    for (int i = 0; i < lines.count(); ++i) {
        const QString &line = lines[i];

        // Code blocks
        if (line.trimmed().startsWith("```")) {
            if (!inCodeBlock) {
                inCodeBlock = true;
                codeBlockLang = line.trimmed().mid(3).trimmed();
                codeBlockContent.clear();
            } else {
                inCodeBlock = false;
                html += "<div style='background-color:#1e1e1e;border-radius:8px;"
                        "padding:12px;margin:8px 0;overflow-x:auto;'>";
                if (!codeBlockLang.isEmpty()) {
                    html += "<div style='color:#888;font-size:11px;margin-bottom:6px;'>"
                            + escapeHtml(codeBlockLang) + "</div>";
                }
                html += "<pre style='margin:0;color:#d4d4d4;font-family:monospace;"
                        "font-size:13px;white-space:pre-wrap;'>"
                        + highlightCode(codeBlockContent, codeBlockLang) + "</pre></div>";
            }
            continue;
        }

        if (inCodeBlock) {
            codeBlockContent += (codeBlockContent.isEmpty() ? "" : "\n") + line;
            continue;
        }

        // Close list if needed
        if (inList && !line.trimmed().startsWith("- ") && !line.trimmed().startsWith("* ") &&
            !QRegularExpression(R"(^\d+\.\s)").match(line.trimmed()).hasMatch()) {
            html += "</ul>";
            inList = false;
        }

        // Empty line
        if (line.trimmed().isEmpty()) {
            html += "<br/>";
            continue;
        }

        // Headers
        if (line.startsWith("### ")) {
            html += "<h3 style='color:#e6e6e6;margin:12px 0 6px 0;'>"
                    + processInlineMarkdown(escapeHtml(line.mid(4))) + "</h3>";
        } else if (line.startsWith("## ")) {
            html += "<h2 style='color:#e6e6e6;margin:14px 0 8px 0;'>"
                    + processInlineMarkdown(escapeHtml(line.mid(3))) + "</h2>";
        } else if (line.startsWith("# ")) {
            html += "<h1 style='color:#e6e6e6;margin:16px 0 10px 0;'>"
                    + processInlineMarkdown(escapeHtml(line.mid(2))) + "</h1>";
        }
        // Horizontal rule
        else if (line.trimmed() == "---" || line.trimmed() == "***") {
            html += "<hr style='border:none;border-top:1px solid #444;margin:12px 0;'/>";
        }
        // Blockquote
        else if (line.trimmed().startsWith("> ")) {
            html += "<blockquote style='border-left:3px solid #58a6ff;padding-left:12px;"
                    "color:#aaa;margin:8px 0;'>"
                    + processInlineMarkdown(escapeHtml(line.trimmed().mid(2))) + "</blockquote>";
        }
        // Unordered list
        else if (line.trimmed().startsWith("- ") || line.trimmed().startsWith("* ")) {
            if (!inList) { html += "<ul style='margin:6px 0;padding-left:20px;'>"; inList = true; }
            html += "<li>" + processInlineMarkdown(escapeHtml(line.trimmed().mid(2))) + "</li>";
        }
        // Ordered list
        else if (QRegularExpression(R"(^\d+\.\s)").match(line.trimmed()).hasMatch()) {
            if (!inList) { html += "<ul style='margin:6px 0;padding-left:20px;'>"; inList = true; }
            QString text = line.trimmed();
            text = text.mid(text.indexOf('.') + 2);
            html += "<li>" + processInlineMarkdown(escapeHtml(text)) + "</li>";
        }
        // Regular paragraph
        else {
            html += "<p style='margin:4px 0;line-height:1.6;'>"
                    + processInlineMarkdown(escapeHtml(line)) + "</p>";
        }
    }

    if (inList) html += "</ul>";
    if (inCodeBlock) {
        // Unclosed code block (streaming) - show as-is
        html += "<div style='background-color:#1e1e1e;border-radius:8px;padding:12px;margin:8px 0;'>"
                "<pre style='margin:0;color:#d4d4d4;font-family:monospace;font-size:13px;"
                "white-space:pre-wrap;'>" + escapeHtml(codeBlockContent) + "</pre></div>";
    }

    return html;
}

QString MarkdownHelper::highlightCode(const QString &code, const QString &lang) const {
    // Basic syntax highlighting with color coding
    QString escaped = escapeHtml(code);

    // Keywords for common languages
    QStringList keywords;
    if (lang == "python" || lang == "py") {
        keywords = {"def", "class", "import", "from", "return", "if", "else", "elif",
                     "for", "while", "try", "except", "with", "as", "in", "not", "and",
                     "or", "True", "False", "None", "self", "lambda", "yield", "async", "await"};
    } else if (lang == "javascript" || lang == "js" || lang == "typescript" || lang == "ts") {
        keywords = {"function", "const", "let", "var", "return", "if", "else", "for",
                     "while", "class", "import", "export", "from", "new", "this", "true",
                     "false", "null", "undefined", "async", "await", "try", "catch"};
    } else if (lang == "cpp" || lang == "c" || lang == "c++") {
        keywords = {"int", "void", "class", "struct", "return", "if", "else", "for",
                     "while", "include", "define", "nullptr", "true", "false", "const",
                     "auto", "virtual", "override", "template", "typename", "namespace"};
    }

    // Highlight keywords
    for (const QString &kw : keywords) {
        escaped.replace(QRegularExpression("\\b(" + QRegularExpression::escape(kw) + ")\\b"),
                        "<span style='color:#569cd6;'>\\1</span>");
    }

    // Highlight strings
    escaped.replace(QRegularExpression(R"((&quot;[^&]*?&quot;))"),
                    "<span style='color:#ce9178;'>\\1</span>");
    escaped.replace(QRegularExpression(R"(('[^']*?'))"),
                    "<span style='color:#ce9178;'>\\1</span>");

    // Highlight comments (// style)
    escaped.replace(QRegularExpression(R"((\/\/.*))"),
                    "<span style='color:#6a9955;'>\\1</span>");

    // Highlight numbers
    escaped.replace(QRegularExpression(R"(\b(\d+\.?\d*)\b)"),
                    "<span style='color:#b5cea8;'>\\1</span>");

    return escaped;
}
