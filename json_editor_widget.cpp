#include "json_editor_widget.h"
#include "json_syntax_highlighter.h"
#include "json_utils.h"
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTextBlock>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QPainter>

JsonEditorWidget::JsonEditorWidget(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 10));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(4 * QFontMetrics(font()).horizontalAdvance(' '));

    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor("#1e1e1e"));
    pal.setColor(QPalette::Text, QColor("#d4d4d4"));
    pal.setColor(QPalette::Highlight, QColor("#264f78"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    setPalette(pal);

    m_gutter = new GutterWidget(this);
    m_gutter->setFoldRegions(&m_foldManager.regions());
    m_gutter->setArrayIndices(&m_arrayIndices);

    m_highlighter = new JsonSyntaxHighlighter(document());

    m_bracketMatchFormat.setBackground(QColor("#264f78"));
    m_bracketMatchFormat.setForeground(QColor("#ffffff"));

    m_errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    m_errorFormat.setUnderlineColor(QColor("#f44747"));

    m_searchFormat.setBackground(QColor("#613214"));
    m_searchFormat.setForeground(QColor("#d4d4d4"));
    m_currentSearchFormat.setBackground(QColor("#f7c325"));
    m_currentSearchFormat.setForeground(QColor("#000000"));

    m_parseTimer = new QTimer(this);
    m_parseTimer->setSingleShot(true);
    m_parseTimer->setInterval(300);
    connect(m_parseTimer, &QTimer::timeout, this, &JsonEditorWidget::parseAndValidate);

    m_foldTimer = new QTimer(this);
    m_foldTimer->setSingleShot(true);
    m_foldTimer->setInterval(200);
    connect(m_foldTimer, &QTimer::timeout, this, [this]() {
        updateFoldRegions();
        updateArrayIndices();
    });

    connect(this, &QPlainTextEdit::updateRequest, this, &JsonEditorWidget::updateGutter);
    connect(this, &QPlainTextEdit::textChanged, this, &JsonEditorWidget::onTextChanged);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &JsonEditorWidget::onCursorPositionChanged);
    connect(this, &QPlainTextEdit::blockCountChanged, this, [this]() {
        if (!m_settingText) updateGutterData();
    });

    connect(m_gutter, &GutterWidget::foldToggleRequested, this, &JsonEditorWidget::toggleFold);

    setViewportMargins(m_gutter->sizeHint().width(), 0, 0, 0);
}

void JsonEditorWidget::loadJson(const QString &text)
{
    m_settingText = true;
    m_foldManager.clearRegions();
    setPlainText(text);
    m_settingText = false;
    parseAndValidate();
    updateFoldRegions();
    updateArrayIndices();
    updateGutterData();
    highlightBrackets();

    // Set cursor to start after all updates
    QTextCursor c(document());
    c.movePosition(QTextCursor::Start);
    setTextCursor(c);
}

void JsonEditorWidget::formatJson()
{
    QString formatted = JsonUtils::formatJson(toPlainText(), 4);
    if (!formatted.isNull()) {
        m_settingText = true;
        m_foldManager.clearRegions();

        QTextCursor cursor(document());
        cursor.select(QTextCursor::Document);
        cursor.insertText(formatted);

        m_settingText = false;
        parseAndValidate();
        updateFoldRegions();
        updateArrayIndices();
        updateGutterData();
        highlightBrackets();
    }
}

void JsonEditorWidget::compactJson()
{
    QString compacted = JsonUtils::compactJson(toPlainText());
    if (!compacted.isNull()) {
        m_settingText = true;
        m_foldManager.clearRegions();
        setPlainText(compacted);
        m_settingText = false;
        parseAndValidate();
        updateFoldRegions();
        updateArrayIndices();
        updateGutterData();
        highlightBrackets();
    }
}

void JsonEditorWidget::foldAll()
{
    m_foldManager.findFoldRegions(document());
    m_foldManager.foldAll(document(), 1);
    updateArrayIndices();
    updateGutterData();
    viewport()->update();
    emit foldStateChanged();
}

void JsonEditorWidget::unfoldAll()
{
    m_foldManager.unfoldAll(document());
    updateArrayIndices();
    updateGutterData();
    viewport()->update();
    emit foldStateChanged();
}

void JsonEditorWidget::toggleFold(int regionId)
{
    FoldRegion *r = m_foldManager.regionById(regionId);
    if (!r) return;

    if (r->isFolded) {
        m_foldManager.unfold(document(), regionId);
    } else {
        m_foldManager.fold(document(), regionId);
    }

    updateArrayIndices();
    updateGutterData();
    viewport()->update();
    emit foldStateChanged();
}

void JsonEditorWidget::copyAll()
{
    QApplication::clipboard()->setText(toPlainText());
}

void JsonEditorWidget::search(const QString &text, bool forward)
{
    if (text != m_searchText) {
        m_searchText = text;
        m_searchResults.clear();
        m_currentSearchIndex = -1;

        if (!text.isEmpty()) {
            QTextDocument *doc = document();
            QTextCursor cursor(doc);
            while (true) {
                cursor = doc->find(text, cursor);
                if (cursor.isNull()) break;
                m_searchResults.append(cursor);
            }
        }
    }

    if (m_searchResults.isEmpty()) return;

    if (forward) {
        m_currentSearchIndex++;
        if (m_currentSearchIndex >= m_searchResults.size())
            m_currentSearchIndex = 0;
    } else {
        m_currentSearchIndex--;
        if (m_currentSearchIndex < 0)
            m_currentSearchIndex = m_searchResults.size() - 1;
    }

    QTextCursor target = m_searchResults[m_currentSearchIndex];
    setTextCursor(target);
    ensureCursorVisible();
    updateSearchHighlights();
}

void JsonEditorWidget::clearSearch()
{
    m_searchText.clear();
    m_searchResults.clear();
    m_currentSearchIndex = -1;
    updateSearchHighlights();
}

void JsonEditorWidget::jumpToSearchResult(int index)
{
    if (index < 0 || index >= m_searchResults.size()) return;

    m_currentSearchIndex = index;

    // Unfold any folded region containing the target line
    QTextBlock block = document()->findBlock(m_searchResults[index].selectionStart());
    m_foldManager.unfoldLine(document(), block.blockNumber());
    updateArrayIndices();
    updateGutterData();
    viewport()->update();

    QTextCursor target = m_searchResults[index];
    setTextCursor(target);
    ensureCursorVisible();
    updateSearchHighlights();
}

QVector<SearchMatch> JsonEditorWidget::getSearchMatches() const
{
    QVector<SearchMatch> matches;
    for (int i = 0; i < m_searchResults.size(); ++i) {
        SearchMatch m;
        QTextBlock block = document()->findBlock(m_searchResults[i].selectionStart());
        m.line = block.blockNumber() + 1;
        m.text = block.text();
        matches.append(m);
    }
    return matches;
}

void JsonEditorWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_gutter->setGeometry(0, cr.top(), m_gutter->width(), cr.height());
}

void JsonEditorWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }
    if (event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier) {
        QString clip = QApplication::clipboard()->text();
        if (!clip.isEmpty()) {
            loadJson(clip);
        }
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void JsonEditorWidget::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);

    if (m_settingText) return;

    // Overlay fold summaries on folded opener lines
    if (m_foldManager.regions().isEmpty()) return;

    QPainter painter(viewport());
    QFontMetrics fm(font());
    QColor bgColor("#1e1e1e");
    QColor summaryColor("#6a9955");

    QTextBlock block = firstVisibleBlock();
    QPointF offset = contentOffset();

    while (block.isValid()) {
        if (!block.isVisible()) {
            block = block.next();
            continue;
        }

        QRectF r = blockBoundingGeometry(block).translated(offset);
        if (r.top() > height())
            break;

        int regionIdx = m_foldManager.regionIndexAtLine(block.blockNumber());
        if (regionIdx >= 0 && m_foldManager.regions()[regionIdx].isFolded) {
            int count = m_foldManager.getItemCount(document(), regionIdx);

            QString lineText = block.text();
            int indent = 0;
            while (indent < lineText.length() && lineText[indent].isSpace())
                indent++;
            QString trimmed = lineText.mid(indent);

            QString summary;
            if (m_foldManager.regions()[regionIdx].isArray()) {
                summary = QString("[%1 items]").arg(count);
            } else {
                summary = QString("{...}");
            }

            // Check if line starts with a key: "key" :
            if (trimmed.startsWith('"')) {
                int endQuote = 1;
                while (endQuote < trimmed.length()) {
                    if (trimmed[endQuote] == '\\') { endQuote += 2; continue; }
                    if (trimmed[endQuote] == '"') { endQuote++; break; }
                    endQuote++;
                }
                // Check for colon after the key
                int colonPos = endQuote;
                while (colonPos < trimmed.length() && trimmed[colonPos].isSpace())
                    colonPos++;
                if (colonPos < trimmed.length() && trimmed[colonPos] == ':') {
                    summary = trimmed.left(endQuote) + " : " + summary;
                }
            }

            QString indentedSummary = lineText.left(indent) + summary;

            int y = (int)r.top();
            int h = (int)r.height();
            int x0 = (int)r.left();

            painter.fillRect(x0, y, viewport()->width(), h, bgColor);

            int textX = x0 + 4;
            int textY = y + fm.ascent() + (h - fm.height()) / 2;

            painter.setPen(summaryColor);
            painter.setFont(font());
            painter.drawText(textX, textY, indentedSummary);
        }

        block = block.next();
    }
}

void JsonEditorWidget::updateGutter(const QRect &rect, int dy)
{
    if (m_settingText) return;
    if (dy)
        m_gutter->scroll(0, dy);
    else
        m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());
    updateGutterData();
}

void JsonEditorWidget::onTextChanged()
{
    if (m_settingText) return;
    // Immediately clear stale fold data — the document has changed
    m_foldManager.clearRegions();
    m_parseTimer->start();
    m_foldTimer->start();
}

void JsonEditorWidget::onCursorPositionChanged()
{
    if (m_settingText) return;
    highlightBrackets();
}

void JsonEditorWidget::parseAndValidate()
{
    QString text = toPlainText();
    if (text.trimmed().isEmpty()) {
        m_valid = true;
        m_errorMsg.clear();
        m_errorLine = 0;
        m_errorCol = 0;
        clearErrorHighlight();
        emit jsonParsed(true, QString(), 0, 0);
        emit statsChanged(0, 0, 0, 0);
        return;
    }

    ParseResult result = JsonUtils::parseWithError(text);
    if (result.valid) {
        m_valid = true;
        m_errorMsg.clear();
        m_errorLine = 0;
        m_errorCol = 0;
        clearErrorHighlight();
        emit jsonParsed(true, QString(), 0, 0);
        emit statsChanged(result.stats.objects, result.stats.arrays, result.stats.keys, result.stats.maxDepth);
    } else {
        m_valid = false;
        m_errorMsg = result.error.msg;
        m_errorLine = result.error.lineno;
        m_errorCol = result.error.colno;
        if (!result.error.hint.isEmpty())
            m_errorMsg += " (" + result.error.hint + ")";

        highlightError(result.error.lineno, result.error.colno);
        emit jsonParsed(false, m_errorMsg, m_errorLine, m_errorCol);
        emit statsChanged(0, 0, 0, 0);
    }
}

void JsonEditorWidget::updateFoldRegions()
{
    m_foldManager.findFoldRegions(document());
    m_gutter->setFoldRegions(&m_foldManager.regions());
}

void JsonEditorWidget::updateArrayIndices()
{
    m_arrayIndices = m_foldManager.getArrayIndices(document());
    m_gutter->setArrayIndices(&m_arrayIndices);
}

void JsonEditorWidget::updateGutterData()
{
    QVector<GutterBlockInfo> blocks;

    QTextBlock block = firstVisibleBlock();
    QPointF offset = contentOffset();

    while (block.isValid()) {
        if (!block.isVisible()) {
            block = block.next();
            continue;
        }

        QRectF r = blockBoundingGeometry(block).translated(offset);
        int top = (int)r.top();
        int height = (int)r.height();

        if (top > this->height())
            break;

        if (top + height >= 0) {
            GutterBlockInfo info;
            info.blockNumber = block.blockNumber();
            info.top = top;
            info.height = height;
            info.visible = true;
            blocks.append(info);
        }

        block = block.next();
    }

    m_gutter->setBlockInfo(blocks, document()->blockCount());
}

void JsonEditorWidget::highlightBrackets()
{
    m_bracketSelections.clear();

    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QTextDocument *doc = document();
    int len = doc->characterCount();

    if (pos < 0 || pos >= len) {
        applyExtraSelections();
        return;
    }

    QChar ch = doc->characterAt(pos);
    QChar matchChar;
    int direction = 0;

    if (ch == '{' || ch == '[' || ch == '(') {
        matchChar = (ch == '{') ? '}' : (ch == '[') ? ']' : ')';
        direction = 1;
    } else if (ch == '}' || ch == ']' || ch == ')') {
        matchChar = (ch == '}') ? '{' : (ch == ']') ? '[' : '(';
        direction = -1;
    } else if (pos > 0) {
        ch = doc->characterAt(pos - 1);
        if (ch == '{' || ch == '[' || ch == '(') {
            matchChar = (ch == '{') ? '}' : (ch == '[') ? ']' : ')';
            direction = 1;
            pos = pos - 1;
        } else if (ch == '}' || ch == ']' || ch == ')') {
            matchChar = (ch == '}') ? '{' : (ch == ']') ? '[' : '(';
            direction = -1;
            pos = pos - 1;
        }
    }

    if (direction == 0) {
        applyExtraSelections();
        return;
    }

    int depth = 0;
    bool inString = false;
    int matchPos = -1;

    if (direction > 0) {
        for (int i = pos + 1; i < len; ++i) {
            QChar c = doc->characterAt(i);
            if (c == '"') {
                int bs = 0;
                int j = i - 1;
                while (j >= 0 && doc->characterAt(j) == '\\') { bs++; j--; }
                if (bs % 2 == 0) inString = !inString;
            } else if (!inString) {
                if (c == ch) depth++;
                else if (c == matchChar) {
                    if (depth == 0) { matchPos = i; break; }
                    depth--;
                }
            }
        }
    } else {
        for (int i = pos - 1; i >= 0; --i) {
            QChar c = doc->characterAt(i);
            if (c == '"') {
                int bs = 0;
                int j = i - 1;
                while (j >= 0 && doc->characterAt(j) == '\\') { bs++; j--; }
                if (bs % 2 == 0) inString = !inString;
            } else if (!inString) {
                if (c == ch) depth++;
                else if (c == matchChar) {
                    if (depth == 0) { matchPos = i; break; }
                    depth--;
                }
            }
        }
    }

    if (matchPos >= 0) {
        QTextEdit::ExtraSelection sel1, sel2;

        QTextCursor c1 = textCursor();
        c1.setPosition(pos);
        c1.setPosition(pos + 1, QTextCursor::KeepAnchor);
        sel1.cursor = c1;
        sel1.format = m_bracketMatchFormat;

        QTextCursor c2 = textCursor();
        c2.setPosition(matchPos);
        c2.setPosition(matchPos + 1, QTextCursor::KeepAnchor);
        sel2.cursor = c2;
        sel2.format = m_bracketMatchFormat;

        m_bracketSelections = {sel1, sel2};
    }

    applyExtraSelections();
}

void JsonEditorWidget::highlightError(int line, int col)
{
    m_errorSelections.clear();
    if (line <= 0) return;

    QTextBlock block = document()->findBlockByNumber(line - 1);
    if (!block.isValid()) return;

    int pos = block.position() + (col > 0 ? col - 1 : 0);

    QTextEdit::ExtraSelection sel;
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(qMin(pos + 1, document()->characterCount() - 1), QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    sel.format = m_errorFormat;

    m_errorSelections = {sel};
    applyExtraSelections();
}

void JsonEditorWidget::clearErrorHighlight()
{
    m_errorSelections.clear();
    setExtraSelections(m_searchSelections + m_bracketSelections);
}

void JsonEditorWidget::updateSearchHighlights()
{
    m_searchSelections.clear();

    for (int i = 0; i < m_searchResults.size(); ++i) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = m_searchResults[i];
        sel.format = (i == m_currentSearchIndex) ? m_currentSearchFormat : m_searchFormat;
        m_searchSelections.append(sel);
    }

    applyExtraSelections();
}

void JsonEditorWidget::applyExtraSelections()
{
    QList<QTextEdit::ExtraSelection> all;
    int total = m_errorSelections.size() + m_searchSelections.size() + m_bracketSelections.size();
    all.reserve(total);
    all.append(m_errorSelections);
    all.append(m_searchSelections);
    all.append(m_bracketSelections);
    setExtraSelections(all);
}