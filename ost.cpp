#include <QApplication>
#include "controller.h"

/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    Controller *controller = new Controller(&a);
    return a.exec();

}



