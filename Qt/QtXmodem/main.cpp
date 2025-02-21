#include "qtxmodem.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtXmodem w;
    w.show();
    return a.exec();
}
