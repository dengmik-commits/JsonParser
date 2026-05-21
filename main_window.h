#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "json_editor_widget.h"
#include "search_bar.h"
#include "search_results_list.h"
#include "status_bar.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void loadFile(const QString &path, const QString &content);

private slots:
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void doFormat();
    void doCompact();
    void doExpandAll();
    void doCollapseAll();
    void doSearch();
    void doCopyAll();
    void doPaste();
    void doClear();

    void onJsonParsed(bool valid, const QString &errorMsg, int errorLine, int errorCol);
    void onStatsChanged(int objects, int arrays, int keys, int depth);
    void onSearchRequested(const QString &text, bool forward);
    void onFindAllRequested(const QString &text);
    void onSearchClosed();

private:
    JsonEditorWidget *m_editor;
    SearchBar *m_searchBar;
    SearchResultsList *m_searchResults;
    StatusBar *m_statusBar;

    QString m_filePath;

    void buildMenuBar();
    void buildToolbar();
    void buildStatusBar();
    void setupShortcuts();

    void saveTo(const QString &path);
};

#endif // MAIN_WINDOW_H