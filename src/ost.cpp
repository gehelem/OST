#include <QGuiApplication>
#include "controller.h"
#include <boost/log/trivial.hpp>


/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
int main(int argc, char *argv[])
{

    BOOST_LOG_TRIVIAL(info) << "OST starting up";
    QGuiApplication app(argc, argv,false);
    QGuiApplication::setOrganizationName("alazob.team");
    QGuiApplication::setApplicationName("ost");

    QCommandLineParser argParser;
    argParser.addHelpOption();

    QCommandLineOption saveAllBlobsOption("s", "Save all received blobs to /tmp");
    QCommandLineOption indiHostOption("host", "INDI server hostname", "host");
    indiHostOption.setDefaultValue("localhost");
    QCommandLineOption indiPortOption("port", "INDI Server port number", "port");
    indiPortOption.setDefaultValue("7624");
    QCommandLineOption webrootOption("webroot", "Web server root folder **must be writable**", "webroot");
    webrootOption.setDefaultValue("/var/www/html");
    QCommandLineOption dbPathOption("dbpath", "DB path", "dbpath");
    dbPathOption.setDefaultValue("");
    QCommandLineOption libPathOption("libpath", "Modules library path", "libpath");
    libPathOption.setDefaultValue("");
    QCommandLineOption installFrontOption("installfront", "Install default webserver structure", "installfront");
    installFrontOption.setDefaultValue("N");
    QCommandLineOption configurationOption("configuration", "Load configuration", "configuration");
    configurationOption.setDefaultValue("default");

    argParser.addOption(saveAllBlobsOption);
    argParser.addOption(indiHostOption);
    argParser.addOption(indiPortOption);
    argParser.addOption(webrootOption);
    argParser.addOption(dbPathOption);
    argParser.addOption(libPathOption);
    argParser.addOption(installFrontOption);
    argParser.addOption(configurationOption);
    argParser.process(app);

    QString hostName = argParser.value(indiHostOption);
    int portNumber = atoi(argParser.value(indiPortOption).toStdString().c_str());
    QString webroot= argParser.value(webrootOption);
    QString dbPath= argParser.value(dbPathOption);
    QString libPath= argParser.value(libPathOption);
    QString installFront= argParser.value(installFrontOption);
    QString conf= argParser.value(configurationOption);

    BOOST_LOG_TRIVIAL(debug) << "INDI Host=" << hostName.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "INDI Port=" << portNumber;
    BOOST_LOG_TRIVIAL(debug) << "Webroot  =" << webroot.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "DB Path  =" << dbPath.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "Modules Library Path  =" << libPath.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "Install front  =" << installFront.toStdString();
    BOOST_LOG_TRIVIAL(debug) << "Load configuration  =" << conf.toStdString();

    Controller controller(
            //&app,
            argParser.isSet("s"),
            hostName,
            portNumber,
            webroot,
            dbPath,
            libPath,
            installFront,
            conf
                );

    Q_UNUSED(controller);

    int nAppReturnCode = QGuiApplication::exec();
    BOOST_LOG_TRIVIAL(info) << "OST app terminated with status : " << nAppReturnCode;
    return nAppReturnCode;

}



