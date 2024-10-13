#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
//#include <QtSql/QSqlError>
#include "qzeroconf.h"

#include "maincontrol.h"

#include "wshandler.h"


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
                   const QString &libpath, const QString &conf, const QString &installfront, const QString &indiserver, const QString &lng);
        ~Controller() override;
    signals:
        void controllerEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                             const QVariantMap &eventData);
    private:
        QString _webroot;
        QString _dbpath;
        QString _libpath;
        QString _installfront;
        QString _conf;
        QString _indiserver;
        QString _lng;
        QVariantMap _availableModuleLibs;
        QStringList _availableIndiDrivers;
        WShandler   *wshandler;
        DBManager   *dbmanager;
        Maincontrol *pMainControl;
        QProcess    *_process;
        QProcess    *_indiProcess;
        QMap<QString, QMap<QString, QString>> mModulesMap;
        QZeroConf zeroConf;
        QString buildName(void);
        QFileSystemWatcher mFileWatcher;
        QStringList mFileList;


        bool loadModule(QString lib, QString name, QString label, QString profile);
        void loadConf(const QString &pConf);
        void saveConf(const QString &pConf);
        void checkModules(void);
        void checkIndiDrivers(void);
        void installFront(void);
        void processOutput();
        void processError();
        void processIndiOutput();
        void processIndiError();
        void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void sendMessage(const QString &pMessage);
        void startPublish();
        void startIndi(void);
        void stopIndi(void);
        void startIndiDriver(const QString &pDriver);
        void stopIndiDriver(const QString &pDriver);

    private slots:
        void OnModuleEvent  (const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        void OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        void OnMainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                            const QVariantMap &pEventData);
        void OnFileWatcherEvent(const QString &pEvent);
        void OnFileChangeEvent(const QString &pEvent);

};
#endif
