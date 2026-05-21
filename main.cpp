#include "main_window.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("JSON 解析器");

    MainWindow window;
    window.show();

    // Load file from command line argument
    if (argc > 1) {
        QString path = QString::fromLocal8Bit(argv[1]);
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString text = in.readAll();
            file.close();
            window.loadFile(path, text);
        }
    }

    return app.exec();
}