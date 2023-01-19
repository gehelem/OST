#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
//#include <QtSql/QSqlError>
#include <boost/log/trivial.hpp>

#include <basemodule.h>

#include "wshandler.h"
#include "dbmanager.h"

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
    Controller(bool saveAllBlobs, const QString& host, int port, const QString& webroot, const QString& dbpath, const QString& libpath, const QString& conf, const QString& installfront);
    ~Controller() override;
signals:
    void controllerEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
private:
    QString _indihost;
    int _indiport;
    QString _webroot;
    QString _dbpath;
    QString _libpath;
    QString _installfront;
    QString _conf;
    QVariantMap _availableModuleLibs;
    WShandler   *wshandler;
    DBManager   *dbmanager;
    QProcess    *_process;

    bool LoadModule(QString lib, QString name, QString label, QString profile);
    void checkModules(void);
    void installFront(void);
    void processOutput();
    void processError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void sendMessage(QString message);

private slots:
    void OnModuleEvent  (const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
    void OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);


};
#endif
