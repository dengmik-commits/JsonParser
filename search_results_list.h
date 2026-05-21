#ifndef SEARCH_RESULTS_LIST_H
#define SEARCH_RESULTS_LIST_H

#include <QListWidget>

struct SearchMatch {
    int line;
    QString text;
};

class SearchResultsList : public QListWidget
{
    Q_OBJECT

public:
    explicit SearchResultsList(QWidget *parent = nullptr);

    void setMatches(const QVector<SearchMatch> &matches);
    void setCurrentIndex(int index);

signals:
    void matchSelected(int index);

private slots:
    void onItemClicked(QListWidgetItem *item);
};

#endif // SEARCH_RESULTS_LIST_H
