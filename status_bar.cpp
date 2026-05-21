#include "status_bar.h"

StatusBar::StatusBar(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 2, 8, 2);
    layout->setSpacing(12);

    m_dotLabel = new QLabel("●");
    m_dotLabel->setFixedSize(16, 16);
    setDotColor("#888888");

    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("color: #d4d4d4;");

    m_statsLabel = new QLabel;
    m_statsLabel->setStyleSheet("color: #858585;");

    m_errorLabel = new QLabel;
    m_errorLabel->setStyleSheet("color: #f44747;");
    m_errorLabel->setWordWrap(true);

    layout->addWidget(m_dotLabel);
    layout->addWidget(m_statusLabel);
    layout->addStretch();
    layout->addWidget(m_statsLabel);
    layout->addStretch();
    layout->addWidget(m_errorLabel);

    setStyleSheet("StatusBar { background: #252526; border-top: 1px solid #3c3c3c; }");
    setFixedHeight(28);
}

void StatusBar::showValid(int objects, int arrays, int keys, int depth)
{
    setDotColor("#4ec9b0");
    m_statusLabel->setText("JSON 有效");
    m_statsLabel->setText(
        QString("对象: %1  数组: %2  键: %3  深度: %4")
            .arg(objects).arg(arrays).arg(keys).arg(depth)
    );
    m_errorLabel->clear();
}

void StatusBar::showError(const QString &msg, int line, int col)
{
    setDotColor("#f44747");
    m_statusLabel->setText("JSON 无效");
    m_statsLabel->clear();
    m_errorLabel->setText(QString("第 %1 行, 第 %2 列: %3").arg(line).arg(col).arg(msg));
}

void StatusBar::showIdle()
{
    setDotColor("#888888");
    m_statusLabel->setText("就绪");
    m_statsLabel->clear();
    m_errorLabel->clear();
}

void StatusBar::setDotColor(const QString &color)
{
    m_dotLabel->setStyleSheet(QString("color: %1; font-size: 14px;").arg(color));
}