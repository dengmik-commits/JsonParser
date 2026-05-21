#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class StatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);

    void showValid(int objects, int arrays, int keys, int depth);
    void showError(const QString &msg, int line, int col);
    void showIdle();

private:
    QLabel *m_dotLabel;
    QLabel *m_statusLabel;
    QLabel *m_statsLabel;
    QLabel *m_errorLabel;

    void setDotColor(const QString &color);
};

#endif // STATUS_BAR_H