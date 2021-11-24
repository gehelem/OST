#include <QApplication>
#include "controller.h"
#include <boost/log/trivial.hpp>


/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    Controller *controller = new Controller(&a);
    Q_UNUSED(controller);
    return a.exec();

}



