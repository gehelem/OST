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
    Controller(bool saveAllBlobs, const QString& host, int port,const QString& webroot,const QString& dbpath);
    ~Controller() override;
signals:
    void controllerEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
private:
    QString _indihost;
    int _indiport;
    QString _webroot;
    QString _dbpath;
    QStringList _availableModuleLibs;
    WShandler   *wshandler;
    DBManager   *dbmanager;
    void LoadModule(QString lib, QString name, QString label, QString profile);
private slots:
    void OnModuleEvent  (const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);
    void OnExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData);


};
#endif
