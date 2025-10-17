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
    QCommandLineOption webrootOption("webroot", "Web server media folder **must be writable**", "webroot");
    webrootOption.setDefaultValue("/var/www/html/ostmedia");
    QCommandLineOption dbPathOption("dbpath", "DB path", "dbpath");
    dbPathOption.setDefaultValue("");
    QCommandLineOption libPathOption("libpath", "Modules library path", "libpath");
    libPathOption.setDefaultValue("");
    QCommandLineOption configurationOption("configuration", "Load configuration", "configuration");
    configurationOption.setDefaultValue("default");
    QCommandLineOption indiServerOption("indiserver", "Start embedded Indi server", "indiserver");
    indiServerOption.setDefaultValue("N");
    QCommandLineOption sslOption("ssl", "Enable SSL for WebSocket server", "ssl");
    sslOption.setDefaultValue("N");
    QCommandLineOption sslCertOption("sslcert", "SSL certificate file path", "sslcert");
    sslCertOption.setDefaultValue("/etc/ost/server.crt");
    QCommandLineOption sslKeyOption("sslkey", "SSL private key file path", "sslkey");
    sslKeyOption.setDefaultValue("/etc/ost/server.key");
    QCommandLineOption lngOption("lng", "Language (en-fr)", "lng");
    lngOption.setDefaultValue("fr");

    argParser.addOption(saveAllBlobsOption);
    argParser.addOption(webrootOption);
    argParser.addOption(dbPathOption);
    argParser.addOption(libPathOption);
    argParser.addOption(configurationOption);
    argParser.addOption(indiServerOption);
    argParser.addOption(sslOption);
    argParser.addOption(sslCertOption);
    argParser.addOption(sslKeyOption);
    argParser.addOption(lngOption);
    argParser.process(app);

    QString webroot = argParser.value(webrootOption);
    QString dbPath = argParser.value(dbPathOption);
    QString libPath = argParser.value(libPathOption);
    QString conf = argParser.value(configurationOption);
    QString indiserver = argParser.value(indiServerOption);
    QString ssl = argParser.value(sslOption);
    QString sslCert = argParser.value(sslCertOption);
    QString sslKey = argParser.value(sslKeyOption);
    QString lng = argParser.value(lngOption);

    sendMessage("Webroot               =" + webroot);
    sendMessage("DB Path               =" + dbPath);
    sendMessage("Modules Library Path  =" + libPath);
    sendMessage("Load configuration    =" + conf);
    sendMessage("Embedded Indi server  =" + indiserver);
    sendMessage("WebSocket SSL         =" + ssl);
    if (ssl != "N")
    {
        sendMessage("SSL Certificate       =" + sslCert);
        sendMessage("SSL Private Key       =" + sslKey);
    }
    sendMessage("Language              =" + lng);

    Controller controller(
        webroot,
        dbPath,
        libPath,
        conf,
        indiserver,
        ssl,
        sslCert,
        sslKey,
        lng
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


