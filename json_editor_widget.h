#ifndef JSON_EDITOR_WIDGET_H
#define JSON_EDITOR_WIDGET_H

#include <QPlainTextEdit>
#include <QMap>
#include <QSet>
#include <QTimer>

#include "fold_region.h"
#include "gutter_widget.h"
#include "search_results_list.h"

class JsonSyntaxHighlighter;

class JsonEditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit JsonEditorWidget(QWidget *parent = nullptr);

    void loadJson(const QString &text);
    void formatJson();
    void compactJson();
    void foldAll();
    void unfoldAll();
    void toggleFold(int regionId);
    void copyAll();

    void search(const QString &text, bool forward = true);
    void clearSearch();
    void jumpToSearchResult(int index);
    QVector<SearchMatch> getSearchMatches() const;

    bool isJsonValid() const { return m_valid; }
    QString errorMessage() const { return m_errorMsg; }
    int errorLine() const { return m_errorLine; }
    int errorCol() const { return m_errorCol; }
    int searchResultCount() const { return m_searchResults.size(); }
    int currentSearchIndex() const { return m_currentSearchIndex; }

    GutterWidget *gutterWidget() const { return m_gutter; }

    const QMap<int, int> &arrayIndices() const { return m_arrayIndices; }

signals:
    void jsonParsed(bool valid, const QString &errorMsg, int errorLine, int errorCol);
    void foldStateChanged();
    void statsChanged(int objects, int arrays, int keys, int depth);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateGutter(const QRect &rect, int dy);
    void onTextChanged();
    void onCursorPositionChanged();

private:
    GutterWidget *m_gutter;
    JsonSyntaxHighlighter *m_highlighter;
    FoldRegionManager m_foldManager;
    QTimer *m_parseTimer;
    QTimer *m_foldTimer;

    bool m_valid = false;
    QString m_errorMsg;
    int m_errorLine = 0;
    int m_errorCol = 0;

    QMap<int, int> m_arrayIndices;

    QList<QTextEdit::ExtraSelection> m_bracketSelections;
    QList<QTextEdit::ExtraSelection> m_errorSelections;
    QList<QTextEdit::ExtraSelection> m_searchSelections;

    QString m_searchText;
    QList<QTextCursor> m_searchResults;
    int m_currentSearchIndex = -1;

    void parseAndValidate();
    void updateFoldRegions();
    void updateArrayIndices();
    void updateGutterData();
    void highlightBrackets();
    void highlightError(int line, int col);
    void clearErrorHighlight();
    void updateSearchHighlights();
    void applyExtraSelections();

    QTextCharFormat m_bracketMatchFormat;
    QTextCharFormat m_errorFormat;
    QTextCharFormat m_searchFormat;
    QTextCharFormat m_currentSearchFormat;
    QRegularExpression m_keyRe;

    bool m_settingText = false;
};

#endif // JSON_EDITOR_WIDGET_H
