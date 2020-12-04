#include <stdio.h>
#include "OSTController.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    OSTController *MyOSTController = new OSTController(&a);
    return a.exec();

}



