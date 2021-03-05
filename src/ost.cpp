#include <QCoreApplication>
#include <boost/log/trivial.hpp>

#include "controller.h"

/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
int main(int argc, char *argv[])
{
    BOOST_LOG_TRIVIAL(info) << "OST starting up";
    QCoreApplication a(argc, argv);
    Controller controller(&a);
    Q_UNUSED(controller);
    int nAppReturnCode = QCoreApplication::exec();
    BOOST_LOG_TRIVIAL(info) << "OST app terminated with status : " << nAppReturnCode;
    return nAppReturnCode;

}



