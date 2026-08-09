#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("SpriteForge"));
    a.setApplicationDisplayName(QStringLiteral("SpriteForge"));

    MainWindow w;
    w.show();
    return a.exec();
}
