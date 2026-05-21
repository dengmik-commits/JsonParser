#include "json_syntax_highlighter.h"

JsonSyntaxHighlighter::JsonSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_keyFormat.setForeground(QColor("#9cdcfe"));
    m_stringFormat.setForeground(QColor("#ce9178"));
    m_numberFormat.setForeground(QColor("#b5cea8"));
    m_boolFormat.setForeground(QColor("#569cd6"));
    m_nullFormat.setForeground(QColor("#569cd6"));
    m_bracketFormat.setForeground(QColor("#ffd700"));
}

void JsonSyntaxHighlighter::highlightBlock(const QString &text)
{
    int len = text.length();
    bool inString = false;
    int stringStart = 0;
    bool lastWasKey = false; // string followed by colon

    for (int i = 0; i < len; ++i) {
        QChar c = text[i];

        if (inString) {
            if (c == '\\') {
                i++; // skip escaped char
                if (i >= len) break;
                continue;
            }
            if (c == '"') {
                inString = false;
                // Check if this string is followed by a colon (it's a key)
                int j = i + 1;
                while (j < len && text[j] == ' ' || text[j] == '\t')
                    j++;
                if (j < len && text[j] == ':') {
                    setFormat(stringStart, i - stringStart + 1, m_keyFormat);
                } else {
                    setFormat(stringStart, i - stringStart + 1, m_stringFormat);
                }
            }
            continue;
        }

        if (c == '"') {
            inString = true;
            stringStart = i;
            continue;
        }

        if (c == '{' || c == '}' || c == '[' || c == ']') {
            setFormat(i, 1, m_bracketFormat);
            continue;
        }

        // Check for true/false/null
        if (c == 't' && i + 3 < len && text[i+1] == 'r' && text[i+2] == 'u' && text[i+3] == 'e') {
            // Make sure it's not part of a longer word
            if ((i == 0 || !text[i-1].isLetter()) && (i+4 >= len || !text[i+4].isLetterOrNumber())) {
                setFormat(i, 4, m_boolFormat);
                i += 3;
                continue;
            }
        }
        if (c == 'f' && i + 4 < len && text[i+1] == 'a' && text[i+2] == 'l' && text[i+3] == 's' && text[i+4] == 'e') {
            if ((i == 0 || !text[i-1].isLetter()) && (i+5 >= len || !text[i+5].isLetterOrNumber())) {
                setFormat(i, 5, m_boolFormat);
                i += 4;
                continue;
            }
        }
        if (c == 'n' && i + 3 < len && text[i+1] == 'u' && text[i+2] == 'l' && text[i+3] == 'l') {
            if ((i == 0 || !text[i-1].isLetter()) && (i+4 >= len || !text[i+4].isLetterOrNumber())) {
                setFormat(i, 4, m_nullFormat);
                i += 3;
                continue;
            }
        }

        // Check for number
        if (c == '-' || c.isDigit()) {
            if (i == 0 || !text[i-1].isLetterOrNumber()) {
                int start = i;
                if (c == '-') i++;
                // digits
                while (i < len && text[i].isDigit()) i++;
                // decimal
                if (i < len && text[i] == '.') {
                    i++;
                    while (i < len && text[i].isDigit()) i++;
                }
                // exponent
                if (i < len && (text[i] == 'e' || text[i] == 'E')) {
                    i++;
                    if (i < len && (text[i] == '+' || text[i] == '-')) i++;
                    while (i < len && text[i].isDigit()) i++;
                }
                // Make sure next char is not alphanumeric (not part of a word)
                if (i >= len || !text[i].isLetterOrNumber()) {
                    setFormat(start, i - start, m_numberFormat);
                }
                // i is already advanced, but the for loop will also advance
                // so we need to go back one
                i--;
                continue;
            }
        }
    }

    // Handle unterminated string at end of line
    if (inString) {
        setFormat(stringStart, len - stringStart, m_stringFormat);
    }
}