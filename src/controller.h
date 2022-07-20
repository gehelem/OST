#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
//#include <QtSql/QSqlError>
#include <boost/log/trivial.hpp>

#include <basemodule.h>

#include "wshandler.h"
#include "maincontrol.h"

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
    Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port,const QString& webroot);
    ~Controller() override;
    WShandler   *wshandler;
public slots:
    void OnNewMessageSent(QString message, QString *pModulename, QString Device);
    void OnLoadModule(QString lib, QString label);
    void OnPropertyChanged(QString *moduleName, QString *propName,QVariant *propValue);

signals:
    void closed();
    void dumpAsked(void);
private:
    void LoadModule(QString lib,QString name,QString label);
    QString _indihost;
    int _indiport;
    QString _webroot;
    QStringList _availableModuleLibs;

};
#endif
