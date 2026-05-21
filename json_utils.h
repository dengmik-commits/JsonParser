#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <QString>
#include <QTextDocument>

struct ErrorInfo {
    QString msg;
    int lineno = 0;
    int colno = 0;
    int pos = 0;
    QString hint;
};

struct JsonStats {
    int objects = 0;
    int arrays = 0;
    int keys = 0;
    int maxDepth = 0;
};

struct ParseResult {
    bool valid;
    ErrorInfo error;
    JsonStats stats;
    QByteArray formattedUtf8; // kept alive for QString references
};

class JsonUtils
{
public:
    static ParseResult parseWithError(const QString &text);
    static QString formatJson(const QString &text, int indent = 4);
    static QString compactJson(const QString &text);
};

#endif // JSON_UTILS_H