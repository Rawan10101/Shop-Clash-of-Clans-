#include "Game.h"
#include "startwindow.h"
#include "shop1.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Shop1 shop2;
    // shop2.show();
    startwindow w;
    w.show();

    return a.exec();
}



