#include "newgenemainwindow.h"
#include <QApplication>
#include "..\..\NewGeneBackEnd\test.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    NewGeneMainWindow w;
    w.show();

    return a.exec();

}
