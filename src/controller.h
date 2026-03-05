#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
//#include <QtSql/QSqlError>
#include "qzeroconf.h"
//#include "maincontrol.h"
#include "wshandler.h"
#include "logger.h"
#include "translatemanager.h"


/*!
 * This class is the heart of OST
 * It dispatches events/orders/datas  from one layer to each other
 * - websocket traffic
 * - functional Modules
 */
class Controller : public QObject
{
        Q_OBJECT
    public:
        Controller(const QString &webroot, const QString &dbpath,
                   const QString &libpath, const QString &conf, const QString &indiserver,
                   const QString &ssl, const QString &sslCert, const QString &sslKey, const QString &lng, const QString &grant,
                   OST::Logger *logger, OST::TranslateManager *translate, const QString &banner);
        ~Controller() override;
    signals:
        void controllerEvent(OST::ExtEvent  event);
    private:
        QString _webroot;
        QString _dbpath;
        QString _libpath;
        QString _conf;
        QString _indiserver;
        QString _lng;
        QString _grant;
        QVariantMap _availableModuleLibs;
        QStringList _availableIndiDrivers;
        WShandler   *wshandler;
        DBManager   *dbmanager;
        OST::Logger *mLogger;
        OST::TranslateManager *mTranslater;
        //Maincontrol *pMainControl;
        QProcess    *_process;
        QProcess    *_indiProcess;
        QMap<QString, QMap<QString, QString>> mModulesMap;
        QZeroConf zeroConf;
        QFileSystemWatcher mFileWatcher;
        QStringList mFilesList;
        QStringList mFoldersList;
        QString mSelectedFolder;
        QString mBanner;

        bool loadModule(QString lib, QString name, QString label, QString profile);
        void loadConf(const QString &pConf);
        void saveConf(const QString &pConf);
        void checkModules(void);
        void checkIndiDrivers(void);
        void processOutput();
        void processError();
        void processIndiOutput();
        void processIndiError();
        void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void startPublish();
        void startIndi(void);
        void stopIndi(void);
        void startIndiDriver(const QString &pDriver);
        void stopIndiDriver(const QString &pDriver);
        void updateGlobalModulesLov(void);
        void logInfo(const QString &message);
        void logInfo(const QString &message, const QVariantList &args);

        QJsonObject getModulesDump(QString clientgrant);
        QString getBanner(void)
        {
            return mBanner;
        }


    private slots:
        void onModuleEvent(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp, OST::LovBase* lov,
                           Datastore* mod);
        void onExternalEvent(OST::ExtEvent event);
        void OnClientEvent(OST::ExtEvent event, QWebSocket* client, QString clientgrant);
        //void OnMainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
        //                    const QVariantMap &pEventData);
        void OnFileWatcherEvent(const QString &pEvent);
        void OnFileChangeEvent(const QString &pEvent);

};
#endif
