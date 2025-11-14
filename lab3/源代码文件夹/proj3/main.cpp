#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

static void configureApplicationStyle()
{
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QFont appFont(QStringLiteral("Microsoft YaHei"), 12);
    QApplication::setFont(appFont);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    configureApplicationStyle();
    MainWindow w;
    w.show();
    return a.exec();
}
