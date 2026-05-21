#ifndef JSON_SYNTAX_HIGHLIGHTER_H
#define JSON_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class JsonSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JsonSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat m_keyFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_boolFormat;
    QTextCharFormat m_nullFormat;
    QTextCharFormat m_bracketFormat;
};

#endif // JSON_SYNTAX_HIGHLIGHTER_H