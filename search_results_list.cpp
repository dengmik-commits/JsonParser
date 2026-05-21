#include "search_results_list.h"
#include <QListWidgetItem>
#include <QScrollBar>

SearchResultsList::SearchResultsList(QWidget *parent)
    : QListWidget(parent)
{
    setMaximumHeight(160);
    setFont(QFont("Consolas", 9));
    setSelectionMode(QAbstractItemView::SingleSelection);

    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor("#252526"));
    pal.setColor(QPalette::Text, QColor("#d4d4d4"));
    pal.setColor(QPalette::Highlight, QColor("#264f78"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    pal.setColor(QPalette::Button, QColor("#3c3c3c"));
    pal.setColor(QPalette::WindowText, QColor("#d4d4d4"));
    setPalette(pal);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Style the scrollbar via the scroll bar widget directly
    verticalScrollBar()->setStyleSheet(
        "QScrollBar { width: 12px; background: #2d2d2d; }"
        "QScrollBar::handle { background: #5a5a5a; border-radius: 6px; min-height: 20px; }"
        "QScrollBar::handle:hover { background: #6a6a6a; }"
        "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }"
        "QScrollBar::add-page, QScrollBar::sub-page { background: none; }"
    );

    setStyleSheet("QListWidget { border: none; border-top: 1px solid #3c3c3c; background: #252526; color: #d4d4d4; }");

    connect(this, &QListWidget::itemClicked, this, &SearchResultsList::onItemClicked);
}

void SearchResultsList::setMatches(const QVector<SearchMatch> &matches)
{
    clear();
    for (int i = 0; i < matches.size(); ++i) {
        QString display = QString("Line %1: %2")
            .arg(matches[i].line)
            .arg(matches[i].text.simplified());
        if (display.length() > 120)
            display.truncate(120);
        QListWidgetItem *item = new QListWidgetItem(display, this);
        item->setData(Qt::UserRole, i);
    }
}

void SearchResultsList::setCurrentIndex(int index)
{
    if (index >= 0 && index < count()) {
        setCurrentRow(index);
        scrollToItem(currentItem());
    }
}

void SearchResultsList::onItemClicked(QListWidgetItem *item)
{
    bool ok;
    int idx = item->data(Qt::UserRole).toInt(&ok);
    if (ok)
        emit matchSelected(idx);
}
