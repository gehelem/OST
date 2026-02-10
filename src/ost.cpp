#include <QGuiApplication>
#include "controller.h"
#include "version.cc"

/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */

int main(int argc, char *argv[])
{

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
    QCommandLineOption grantOption("grant", "Security level (0 : disabled - 1 : Read only - 2 : user grants)", "grant");
    lngOption.setDefaultValue("fr");
    QCommandLineOption logLevel("loglevel", "Log level : 0:debug // 4:critical", "loglevel");
    logLevel.setDefaultValue("1");
    QString filename = "ostserver_" + QDateTime().currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".log";
    QCommandLineOption logFileOption("logfile", "Logfile path (default ostserver-(current date).log)", "logfile", filename);

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
    argParser.addOption(grantOption);
    argParser.addOption(logLevel);
    argParser.addOption(logFileOption);
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
    QString grant = argParser.value(grantOption);
    int loglevel = argParser.value(logLevel).toInt();
    QString logfile = argParser.value(logFileOption);

    OST::Logger mLogger;
    OST::TranslateManager mTranslater;
    mLogger.setFileLogging(true, logfile);
    mLogger.setLogLevel(OST::LogLevel::Info);

    mLogger.info("OST starting up, ladies and gentlemen, squeeze your ALZ !");
    mLogger.info("                                  ");
    mLogger.info("    *******    ******** **********");
    mLogger.info("   **/////**  **////// /////**/// ");
    mLogger.info("  **     //**/**           /**    ");
    mLogger.info(" /**      /**/*********    /**    ");
    mLogger.info(" /**      /**////////**    /**    ");
    mLogger.info(" //**     **        /**    /**    ");
    mLogger.info("  //*******   ********     /**    ");
    mLogger.info("   ///////   ////////      //     ");
    mLogger.info("                                  ");
    mLogger.info("Git Hash              =" + QString::fromStdString(Version::GIT_SHA1));
    mLogger.info("Git Date              =" + QString::fromStdString(Version::GIT_DATE));
    mLogger.info("Git Commit subject    =" + QString::fromStdString(Version::GIT_COMMIT_SUBJECT));

    mLogger.info("Webroot               =" + webroot);
    mLogger.info("DB Path               =" + dbPath);
    mLogger.info("Modules Library Path  =" + libPath);
    mLogger.info("Load configuration    =" + conf);
    mLogger.info("Embedded Indi server  =" + indiserver);
    mLogger.info("WebSocket SSL         =" + ssl);
    if (ssl != "N")
    {
        mLogger.info("SSL Certificate       =" + sslCert);
        mLogger.info("SSL Private Key       =" + sslKey);
    }
    mLogger.info("Language              =" + lng);
    mLogger.info("Security              =" + grant);
    mLogger.info("Log file              =" + logfile);
    mLogger.info("Log level             =" + QString::number(loglevel));

    mLogger.setLogLevel(static_cast<OST::LogLevel>(loglevel));

    Controller controller(
        webroot,
        dbPath,
        libPath,
        conf,
        indiserver,
        ssl,
        sslCert,
        sslKey,
        lng,
        grant,
        &mLogger,
        &mTranslater
    );

    Q_UNUSED(controller);

    int nAppReturnCode = QGuiApplication::exec();
    mLogger.info("OST app terminated with status : " + QString(nAppReturnCode));
    return nAppReturnCode;

}

