#include "json_utils.h"
#include "json.hpp"
#include <QRegularExpression>
#include <QTextBlock>

using json = nlohmann::json;

static JsonStats computeStats(const json &j, int depth = 0)
{
    JsonStats s;
    s.maxDepth = depth;

    if (j.is_object()) {
        s.objects = 1;
        s.keys = (int)j.size();
        for (auto it = j.begin(); it != j.end(); ++it) {
            JsonStats child = computeStats(it.value(), depth + 1);
            s.objects += child.objects;
            s.arrays += child.arrays;
            s.keys += child.keys;
            s.maxDepth = qMax(s.maxDepth, child.maxDepth);
        }
    } else if (j.is_array()) {
        s.arrays = 1;
        for (const auto &item : j) {
            JsonStats child = computeStats(item, depth + 1);
            s.objects += child.objects;
            s.arrays += child.arrays;
            s.keys += child.keys;
            s.maxDepth = qMax(s.maxDepth, child.maxDepth);
        }
    }

    return s;
}

static ErrorInfo buildErrorInfo(const QString &text, const std::string &errorMsg, int bytePos)
{
    ErrorInfo err;
    err.msg = QString::fromUtf8(errorMsg);
    err.pos = bytePos;

    // Convert byte offset to line/column using the original UTF-8
    int line = 1, col = 1;
    for (int i = 0; i < bytePos && i < text.length(); ++i) {
        if (text[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    err.lineno = line;
    err.colno = col;

    return err;
}

ParseResult JsonUtils::parseWithError(const QString &text)
{
    ParseResult result;
    QByteArray utf8 = text.toUtf8();
    result.formattedUtf8 = utf8; // keep alive

    try {
        json j = json::parse(utf8.constData());
        result.valid = true;
        result.stats = computeStats(j);
    } catch (const json::exception &e) {
        result.valid = false;
        result.error = buildErrorInfo(text, e.what(), -1);

        // Try to extract byte position from nlohmann error message
        // Format: "[json.exception.parse_error.101] parse error at 123: ..."
        QString msg = QString::fromUtf8(e.what());
        QRegularExpression posRe(R"(at\s+(\d+))");
        QRegularExpressionMatch m = posRe.match(msg);
        if (m.hasMatch()) {
            int bytePos = m.captured(1).toInt();
            result.error = buildErrorInfo(text, e.what(), bytePos);
        }
    }

    return result;
}

QString JsonUtils::formatJson(const QString &text, int indent)
{
    try {
        QByteArray utf8 = text.toUtf8();
        json j = json::parse(utf8.constData());
        std::string formatted = j.dump(indent);
        return QString::fromUtf8(formatted.c_str());
    } catch (...) {
        return QString();
    }
}

QString JsonUtils::compactJson(const QString &text)
{
    try {
        QByteArray utf8 = text.toUtf8();
        json j = json::parse(utf8.constData());
        std::string compacted = j.dump();
        return QString::fromUtf8(compacted.c_str());
    } catch (...) {
        return QString();
    }
}
