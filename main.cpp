#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    qApp->setStyle(QStyleFactory::create("Fusion"));      
    QFontDatabase::addApplicationFont(":/resource/fonts/Ubuntu.ttf");
    QFont fontUbuntu = QFont("Ubuntu Condensed",11);
    fontUbuntu.setStyleStrategy(QFont::PreferAntialias);
    QApplication::setFont(fontUbuntu);

    w.show();

    return a.exec();
}
