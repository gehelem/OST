#include <QGuiApplication>
#include "controller.h"
#include "version.cc"

/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */
void sendMessage(const QString &pMessage);

int main(int argc, char *argv[])
{

    sendMessage("OST starting up, ladies and gentlemen, squeeze your ALZ !");
    sendMessage("                                  ");
    sendMessage("    *******    ******** **********");
    sendMessage("   **/////**  **////// /////**/// ");
    sendMessage("  **     //**/**           /**    ");
    sendMessage(" /**      /**/*********    /**    ");
    sendMessage(" /**      /**////////**    /**    ");
    sendMessage(" //**     **        /**    /**    ");
    sendMessage("  //*******   ********     /**    ");
    sendMessage("   ///////   ////////      //     ");
    sendMessage("                                  ");

    sendMessage("Git Hash              =" + QString::fromStdString(Version::GIT_SHA1));
    sendMessage("Git Date              =" + QString::fromStdString(Version::GIT_DATE));
    sendMessage("Git Commit subject    =" + QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    QGuiApplication app(argc, argv, false);
    QGuiApplication::setOrganizationName("alazob.team");
    QGuiApplication::setApplicationName("ost");

    QCommandLineParser argParser;
    argParser.addHelpOption();

    QCommandLineOption saveAllBlobsOption("s", "Save all received blobs to /tmp");
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
    argParser.addOption(webrootOption);
    argParser.addOption(dbPathOption);
    argParser.addOption(libPathOption);
    argParser.addOption(installFrontOption);
    argParser.addOption(configurationOption);
    argParser.process(app);

    QString webroot = argParser.value(webrootOption);
    QString dbPath = argParser.value(dbPathOption);
    QString libPath = argParser.value(libPathOption);
    QString installFront = argParser.value(installFrontOption);
    QString conf = argParser.value(configurationOption);

    sendMessage("Webroot               =" + webroot);
    sendMessage("DB Path               =" + dbPath);
    sendMessage("Modules Library Path  =" + libPath);
    sendMessage("Install front         =" + installFront);
    sendMessage("Load configuration    =" + conf);

    Controller controller(
        webroot,
        dbPath,
        libPath,
        installFront,
        conf
    );

    Q_UNUSED(controller);

    int nAppReturnCode = QGuiApplication::exec();
    sendMessage("OST app terminated with status : " + QString(nAppReturnCode));
    return nAppReturnCode;

}

void sendMessage(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}


