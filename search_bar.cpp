#include "search_bar.h"
#include <QKeyEvent>
#include <QStyle>

class SearchLineEdit : public QLineEdit
{
public:
    SearchLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}
protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Escape) {
            clear();
            static_cast<SearchBar *>(parentWidget())->onSearchTextChanged(QString());
        } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            if (e->modifiers() & Qt::ShiftModifier)
                static_cast<SearchBar *>(parentWidget())->onPrevClicked();
            else
                static_cast<SearchBar *>(parentWidget())->onNextClicked();
        } else {
            QLineEdit::keyPressEvent(e);
        }
    }
};

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(32);

    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor("#3c3c3c"));
    pal.setColor(QPalette::Text, QColor("#d4d4d4"));
    pal.setColor(QPalette::Button, QColor("#3c3c3c"));
    pal.setColor(QPalette::ButtonText, QColor("#d4d4d4"));
    setPalette(pal);
    setAutoFillBackground(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    m_edit = new SearchLineEdit(this);
    m_edit->setPlaceholderText("搜索...");
    m_edit->setMaximumWidth(300);
    m_edit->setMinimumWidth(120);
    connect(m_edit, &QLineEdit::textChanged, this, &SearchBar::onSearchTextChanged);

    m_prevBtn = new QPushButton(u8"▲", this);
    m_prevBtn->setFixedSize(24, 24);
    m_prevBtn->setToolTip("上一个 (Shift+Enter)");
    connect(m_prevBtn, &QPushButton::clicked, this, &SearchBar::onPrevClicked);

    m_nextBtn = new QPushButton(u8"▼", this);
    m_nextBtn->setFixedSize(24, 24);
    m_nextBtn->setToolTip("下一个 (Enter)");
    connect(m_nextBtn, &QPushButton::clicked, this, &SearchBar::onNextClicked);

    m_findAllBtn = new QPushButton(u8"全部", this);
    m_findAllBtn->setFixedSize(36, 24);
    m_findAllBtn->setToolTip("列出所有匹配项");
    connect(m_findAllBtn, &QPushButton::clicked, this, &SearchBar::onFindAllClicked);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setMinimumWidth(60);

    m_closeBtn = new QPushButton(u8"✕", this);
    m_closeBtn->setFixedSize(24, 24);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        emit clearRequested();
        emit closeRequested();
    });

    layout->addWidget(m_edit);
    layout->addWidget(m_prevBtn);
    layout->addWidget(m_nextBtn);
    layout->addWidget(m_findAllBtn);
    layout->addWidget(m_infoLabel);
    layout->addStretch();
    layout->addWidget(m_closeBtn);

    hide();
}

QString SearchBar::searchText() const { return m_edit->text(); }

void SearchBar::setSearchText(const QString &text) { m_edit->setText(text); }

void SearchBar::setMatchInfo(int current, int total)
{
    if (total == 0 && m_edit->text().isEmpty())
        m_infoLabel->clear();
    else
        m_infoLabel->setText(QString("%1/%2").arg(current).arg(total));
}

void SearchBar::focusSearch()
{
    show();
    m_edit->setFocus();
    m_edit->selectAll();
}

void SearchBar::onSearchTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emit clearRequested();
    } else {
        emit searchRequested(text, true);
    }
}

void SearchBar::onNextClicked()
{
    if (!m_edit->text().isEmpty())
        emit searchRequested(m_edit->text(), true);
}

void SearchBar::onPrevClicked()
{
    if (!m_edit->text().isEmpty())
        emit searchRequested(m_edit->text(), false);
}

void SearchBar::onFindAllClicked()
{
    if (!m_edit->text().isEmpty())
        emit findAllRequested(m_edit->text());
}
