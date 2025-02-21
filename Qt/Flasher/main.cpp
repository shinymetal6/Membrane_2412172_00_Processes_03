#include "flasher.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Flasher w;
    w.show();
    return a.exec();
}
