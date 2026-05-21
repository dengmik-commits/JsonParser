#ifndef SEARCH_BAR_H
#define SEARCH_BAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString &text);
    void setMatchInfo(int current, int total);
    void focusSearch();

signals:
    void searchRequested(const QString &text, bool forward);
    void findAllRequested(const QString &text);
    void clearRequested();
    void closeRequested();

public slots:
    void onSearchTextChanged(const QString &text);
    void onNextClicked();
    void onPrevClicked();
    void onFindAllClicked();

private:
    QLineEdit *m_edit;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_findAllBtn;
    QPushButton *m_closeBtn;
    QLabel *m_infoLabel;
};

#endif // SEARCH_BAR_H