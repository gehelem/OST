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
    QCoreApplication app(argc, argv);

    QCommandLineParser argParser;
    argParser.addHelpOption();
    QCommandLineOption saveAllBlobsOption("s", "Save all received blobs to /tmp");
    argParser.addOption(saveAllBlobsOption);
    argParser.process(app);

    Controller controller(&app, argParser.isSet("s"));
    Q_UNUSED(controller);

    int nAppReturnCode = QCoreApplication::exec();
    BOOST_LOG_TRIVIAL(info) << "OST app terminated with status : " << nAppReturnCode;
    return nAppReturnCode;
}
