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
                   const QString &libpath, const QString &conf, const QString &installfront);
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
        QVariantMap _availableModuleLibs;
        WShandler   *wshandler;
        DBManager   *dbmanager;
        Maincontrol *pMainControl;
        QProcess    *_process;
        QMap<QString, QMap<QString, QString>> mModulesMap;
        QZeroConf zeroConf;
        QString buildName(void);


        bool loadModule(QString lib, QString name, QString label, QString profile);
        void loadConf(const QString &pConf);
        void saveConf(const QString &pConf);
        void checkModules(void);
        void installFront(void);
        void processOutput();
        void processError();
        void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void sendMessage(const QString &pMessage);
        void startPublish();

    private slots:
        void OnModuleEvent  (const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        void OnExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                             const QVariantMap &pEventData);
        void OnMainCtlEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                            const QVariantMap &pEventData);

};
#endif
