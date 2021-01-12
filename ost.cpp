#include <stdio.h>
#include "controller.h"
#include <QApplication>
#include <QMetaProperty>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    Controller *controller = new Controller(&a);
    return a.exec();

}



