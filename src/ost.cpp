#include <QCoreApplication>
#include "controller.h"
#include <boost/log/trivial.hpp>


/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
int main(int argc, char *argv[])
{

    BOOST_LOG_TRIVIAL(info) << "OST starting up";
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("alazob.team");
    QCoreApplication::setApplicationName("ost");

    QCommandLineParser argParser;
    argParser.addHelpOption();

    QCommandLineOption saveAllBlobsOption("s", "Save all received blobs to /tmp");
    QCommandLineOption indiHostOption("host", "INDI server hostname", "host");
    indiHostOption.setDefaultValue("localhost");
    QCommandLineOption indiPortOption("port", "INDI Server port number", "port");
    indiPortOption.setDefaultValue("7624");
    QCommandLineOption webrootOption("webroot", "Web server root folder", "webroot");
    webrootOption.setDefaultValue("/var/www/html");

    argParser.addOption(saveAllBlobsOption);
    argParser.addOption(indiHostOption);
    argParser.addOption(indiPortOption);
    argParser.addOption(webrootOption);
    argParser.process(app);

    QString hostName = argParser.value(indiHostOption);
    int portNumber = atoi(argParser.value(indiPortOption).toStdString().c_str());
    QString webroot= argParser.value(webrootOption);

    BOOST_LOG_TRIVIAL(debug) << "INDI Host=" << hostName.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "INDI Port=" << portNumber;
    BOOST_LOG_TRIVIAL(debug) << "Webroot  =" << webroot.toStdString();

    Controller controller(
            &app,
            argParser.isSet("s"),
            hostName,
            portNumber,
            webroot);
    Q_UNUSED(controller);

    int nAppReturnCode = QCoreApplication::exec();
    BOOST_LOG_TRIVIAL(info) << "OST app terminated with status : " << nAppReturnCode;
    return nAppReturnCode;

}



