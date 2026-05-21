#include "main_window.h"
#include "json_utils.h"
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QShortcut>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("JSON 解析器");
    resize(1000, 700);
    setWindowIcon(QIcon(":/app.ico"));

    // Central widget
    QWidget *central = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_editor = new JsonEditorWidget;
    m_searchBar = new SearchBar;
    m_searchResults = new SearchResultsList(this);
    m_searchResults->hide();
    m_statusBar = new StatusBar;

    layout->addWidget(m_editor);
    layout->addWidget(m_searchResults);
    layout->addWidget(m_searchBar);
    layout->addWidget(m_statusBar);

    setCentralWidget(central);

    // Connections
    connect(m_editor, &JsonEditorWidget::jsonParsed, this, &MainWindow::onJsonParsed);
    connect(m_editor, &JsonEditorWidget::statsChanged, this, &MainWindow::onStatsChanged);
    connect(m_searchBar, &SearchBar::searchRequested, this, &MainWindow::onSearchRequested);
    connect(m_searchBar, &SearchBar::findAllRequested, this, &MainWindow::onFindAllRequested);
    connect(m_searchBar, &SearchBar::clearRequested, this, [this]() {
        m_editor->clearSearch();
        m_searchResults->hide();
    });
    connect(m_searchBar, &SearchBar::closeRequested, this, &MainWindow::onSearchClosed);

    connect(m_searchResults, &SearchResultsList::matchSelected, this, [this](int index) {
        m_editor->jumpToSearchResult(index);
    });

    buildMenuBar();
    buildToolbar();
    setupShortcuts();

    // Apply dark theme to main window
    setStyleSheet(R"(
        QMainWindow { background: #1e1e1e; }
        QMenuBar { background: #2d2d2d; color: #d4d4d4; border-bottom: 1px solid #3c3c3c; }
        QMenuBar::item:selected { background: #505050; }
        QMenu { background: #2d2d2d; color: #d4d4d4; border: 1px solid #3c3c3c; }
        QMenu::item:selected { background: #505050; }
        QToolBar { background: #2d2d2d; border-bottom: 1px solid #3c3c3c; spacing: 4px; padding: 2px; }
        QToolButton { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; padding: 4px 8px; border-radius: 3px; }
        QToolButton:hover { background: #505050; }
        QToolTip { background: #2d2d2d; color: #d4d4d4; border: 1px solid #555; }
    )");
}

void MainWindow::buildMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // 文件 menu
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("打开(&O)", this, &MainWindow::fileOpen, QKeySequence::Open);
    fileMenu->addAction("保存(&S)", this, &MainWindow::fileSave, QKeySequence::Save);
    fileMenu->addAction("另存为(&A)...", this, &MainWindow::fileSaveAs, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)", this, &QWidget::close, QKeySequence::Quit);

    // 编辑 menu
    QMenu *editMenu = menuBar->addMenu("编辑(&E)");
    editMenu->addAction("格式化(&F)", this, &MainWindow::doFormat, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    editMenu->addAction("压缩(&C)", this, &MainWindow::doCompact, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    editMenu->addSeparator();
    editMenu->addAction("全部展开(&E)", this, &MainWindow::doExpandAll, QKeySequence(Qt::CTRL | Qt::Key_E));
    editMenu->addAction("全部折叠(&W)", this, &MainWindow::doCollapseAll, QKeySequence(Qt::CTRL | Qt::Key_W));
    editMenu->addSeparator();
    editMenu->addAction("搜索(&S)", this, &MainWindow::doSearch, QKeySequence::Find);
    editMenu->addSeparator();
    editMenu->addAction("粘贴(&V)", this, &MainWindow::doPaste, QKeySequence::Paste);
    editMenu->addAction("复制全部(&A)", this, &MainWindow::doCopyAll, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    editMenu->addSeparator();
    editMenu->addAction("清空(&L)", this, &MainWindow::doClear);
}

void MainWindow::buildToolbar()
{
    QToolBar *toolbar = addToolBar("工具");
    toolbar->setMovable(false);
    toolbar->setFloatable(false);

    toolbar->addAction("打开", this, &MainWindow::fileOpen);
    toolbar->addAction("保存", this, &MainWindow::fileSave);
    toolbar->addSeparator();
    toolbar->addAction("格式化", this, &MainWindow::doFormat);
    toolbar->addAction("压缩", this, &MainWindow::doCompact);
    toolbar->addSeparator();
    toolbar->addAction("全部展开", this, &MainWindow::doExpandAll);
    toolbar->addAction("全部折叠", this, &MainWindow::doCollapseAll);
    toolbar->addSeparator();
    toolbar->addAction("搜索", this, &MainWindow::doSearch);
    toolbar->addSeparator();
    toolbar->addAction("复制全部", this, &MainWindow::doCopyAll);
    toolbar->addAction("粘贴", this, &MainWindow::doPaste);
    toolbar->addSeparator();
    toolbar->addAction("清空", this, &MainWindow::doClear);
}

void MainWindow::setupShortcuts()
{
    // Additional shortcuts beyond menu shortcuts
    new QShortcut(QKeySequence(Qt::Key_Escape), this, [this]() {
        if (m_searchBar->isVisible()) {
            m_searchBar->hide();
            m_editor->clearSearch();
            m_searchResults->hide();
        }
    });
}

void MainWindow::fileOpen()
{
    QString path = QFileDialog::getOpenFileName(this, "打开 JSON 文件", QString(),
                                                 "JSON 文件 (*.json);;所有文件 (*.*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + path);
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString text = in.readAll();
    file.close();

    m_filePath = path;
    m_editor->loadJson(text);
    setWindowTitle("JSON 解析器 - " + QFileInfo(path).fileName());
}

void MainWindow::fileSave()
{
    if (m_filePath.isEmpty()) {
        fileSaveAs();
        return;
    }
    saveTo(m_filePath);
}

void MainWindow::fileSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, "保存 JSON 文件", QString(),
                                                 "JSON 文件 (*.json);;所有文件 (*.*)");
    if (path.isEmpty()) return;
    saveTo(path);
}

void MainWindow::saveTo(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件: " + path);
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_editor->toPlainText();
    file.close();

    m_filePath = path;
    setWindowTitle("JSON 解析器 - " + QFileInfo(path).fileName());
}

void MainWindow::doFormat()
{
    m_editor->formatJson();
}

void MainWindow::doCompact()
{
    m_editor->compactJson();
}

void MainWindow::doExpandAll()
{
    m_editor->unfoldAll();
}

void MainWindow::doCollapseAll()
{
    m_editor->foldAll();
}

void MainWindow::doSearch()
{
    m_searchBar->show();
    m_searchBar->focusSearch();
}

void MainWindow::doCopyAll()
{
    m_editor->copyAll();
}

void MainWindow::doPaste()
{
    QString text = QApplication::clipboard()->text();
    if (!text.isEmpty()) {
        m_editor->loadJson(text);
    }
}

void MainWindow::loadFile(const QString &path, const QString &content)
{
    m_filePath = path;
    m_editor->loadJson(content);
    setWindowTitle("JSON 解析器 - " + QFileInfo(path).fileName());
}

void MainWindow::doClear()
{
    m_editor->loadJson("");
    m_filePath.clear();
    setWindowTitle("JSON 解析器");
}

void MainWindow::onJsonParsed(bool valid, const QString &errorMsg, int errorLine, int errorCol)
{
    Q_UNUSED(errorMsg)
    if (valid) {
        // Stats will be updated via onStatsChanged
    } else {
        m_statusBar->showError(errorMsg, errorLine, errorCol);
    }
}

void MainWindow::onStatsChanged(int objects, int arrays, int keys, int depth)
{
    if (m_editor->isJsonValid()) {
        m_statusBar->showValid(objects, arrays, keys, depth);
    }
}

void MainWindow::onSearchRequested(const QString &text, bool forward)
{
    m_editor->search(text, forward);
    m_searchBar->setMatchInfo(m_editor->currentSearchIndex() + 1, m_editor->searchResultCount());
}

void MainWindow::onFindAllRequested(const QString &text)
{
    m_editor->search(text, true);
    m_searchBar->setMatchInfo(m_editor->currentSearchIndex() + 1, m_editor->searchResultCount());

    // Populate results list
    QVector<SearchMatch> matches = m_editor->getSearchMatches();
    m_searchResults->setMatches(matches);
    m_searchResults->show();
    m_searchResults->setCurrentIndex(m_editor->currentSearchIndex());
}

void MainWindow::onSearchClosed()
{
    m_editor->clearSearch();
    m_searchResults->hide();
}