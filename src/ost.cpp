#include <QGuiApplication>
#include "controller.h"
#include "version.cc"
#include "model/element/common.h"

/*!
 * Entry point
 * Should become some kind of service to start & respawn with host
 */

int main(int argc, char *argv[])
{
    qDebug() << "MAIN 0";

    // Parse arguments before constructing QGuiApplication: on an unknown
    // option or --help, we want a plain `return` from main() with no
    // QGuiApplication ever having existed. Constructing one only to have
    // it torn down almost immediately (no event loop ever run) reliably
    // corrupted the heap on exit ("double free or corruption") - true even
    // with a clean return letting destructors run normally, so it's not
    // about skipping destructors, something about this app's very early
    // teardown itself is the problem. Simplest robust fix: don't create it
    // at all on an early-exit path.
    QStringList cliArgs;
    for (int i = 0; i < argc; ++i) cliArgs << QString::fromLocal8Bit(argv[i]);

    QCommandLineParser argParser;
    argParser.addHelpOption();

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

    QCommandLineOption bannerOption("banner", "Frontend banner", "banner");
    bannerOption.setDefaultValue("Observatoire Sans tête");
    QCommandLineOption setAdminPasswordOption("setadminpassword", "Set admin password", "setadminpassword");
    setAdminPasswordOption.setDefaultValue("");
    QCommandLineOption systemWatchIntervalOption("systemwatchinterval", "System watch interval (s)", "systemwatchinterval");
    systemWatchIntervalOption.setDefaultValue("10");
    QCommandLineOption minFreePercentOption("minfree", "Minimum free disk space before skipping image write (%, 0-99)", "minfree");
    minFreePercentOption.setDefaultValue("10");


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
    argParser.addOption(bannerOption);
    argParser.addOption(setAdminPasswordOption);
    argParser.addOption(systemWatchIntervalOption);
    argParser.addOption(minFreePercentOption);
    if (!argParser.parse(cliArgs))
    {
        fputs(qPrintable(argParser.errorText()), stderr);
        fputs("\n", stderr);
        return 1;
    }
    if (argParser.isSet("help"))
    {
        fputs(qPrintable(argParser.helpText()), stdout);
        return 0;
    }

    // Only now, past every early-exit path, construct the application.
    QGuiApplication app(argc, argv, false);
    QGuiApplication::setOrganizationName("alazob.team");
    QGuiApplication::setApplicationName("ost");
    // Initialize Qt embedded resources (must be done before using them)
    Q_INIT_RESOURCE(translations);
    // Register custom types for queued signal/slot connections
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::EvType>("OST::EvType");

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
    QString banner = argParser.value(bannerOption);
    QString setAdminPassword = argParser.value(setAdminPasswordOption);
    int systemWatchInterval = argParser.value(systemWatchIntervalOption).toInt();
    int minFreePercent      = qBound(0, argParser.value(minFreePercentOption).toInt(), 99);

    OST::Logger mLogger;
    OST::TranslateManager mTranslater;
    mTranslater.loadLanguages({"fr", "en", "de", "it", "es"});
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
    // %1 placeholders here, not string concatenation - startup logging used
    // to bake the actual runtime value (git hash, log filename, ...)
    // straight into the logged QString, so the tool that scans the server's
    // own log history for translatable strings picked up a different
    // "source" text every single run (a timestamped log filename is never
    // the same twice) instead of one stable, reusable template - Gilles,
    // 2026-10-04, spotting one such entry in a translations_pending_*.ts:
    // "on peut generaliser a toutes les mLogger.info() qui sont utilisees
    // sans argument". QString::arg() keeps the template stable across runs
    // (translate it once) while still substituting the real value.
    mLogger.info(QString("Git Hash              =%1").arg(QString::fromStdString(Version::GIT_SHA1)));
    mLogger.info(QString("Git Date              =%1").arg(QString::fromStdString(Version::GIT_DATE)));
    mLogger.info(QString("Git Commit subject    =%1").arg(QString::fromStdString(Version::GIT_COMMIT_SUBJECT)));
    mLogger.info(QString("Git Tag               =%1").arg(QString::fromStdString(Version::GIT_TAG)));

    mLogger.info(QString("Webroot               =%1").arg(webroot));
    mLogger.info(QString("DB Path               =%1").arg(dbPath));
    mLogger.info(QString("Modules Library Path  =%1").arg(libPath));
    mLogger.info(QString("Load configuration    =%1").arg(conf));
    mLogger.info(QString("Embedded Indi server  =%1").arg(indiserver));
    mLogger.info(QString("WebSocket SSL         =%1").arg(ssl));
    if (ssl != "N")
    {
        mLogger.info(QString("SSL Certificate       =%1").arg(sslCert));
        mLogger.info(QString("SSL Private Key       =%1").arg(sslKey));
    }
    mLogger.info(QString("Language              =%1").arg(lng));
    mLogger.info(QString("Security              =%1").arg(grant));
    mLogger.info(QString("Log file              =%1").arg(logfile));
    mLogger.info(QString("Log level             =%1").arg(loglevel));
    mLogger.info(QString("Banner                =%1").arg(banner));
    mLogger.info(QString("System watch interval =%1s (0 to disable)").arg(systemWatchInterval));
    mLogger.info(QString("Min free disk space   =%1%").arg(minFreePercent));
    if (setAdminPassword != "") mLogger.info("Set ADMIN password    = [hidden]");

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
        &mTranslater,
        banner,
        setAdminPassword,
        QString::fromStdString(Version::GIT_SHA1),
        QString::fromStdString(Version::GIT_DATE),
        QString::fromStdString(Version::GIT_COMMIT_SUBJECT),
        QString::fromStdString(Version::GIT_TAG),
        systemWatchInterval,
        minFreePercent
    );

    Q_UNUSED(controller);

    int nAppReturnCode = QGuiApplication::exec();
    mLogger.info(QString("OST app terminated with status : %1").arg(nAppReturnCode));
    return nAppReturnCode;

}

