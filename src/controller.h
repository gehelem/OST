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
    Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port);
    ~Controller() override;
    WShandler   *wshandler;
public slots:
    void OnValueChanged(double newValue);
    //void OnValueChanged(QString newValue);
    void OnPropertyCreated(Property *pProperty);

signals:
    void closed();
private:
    void LoadModule(QString lib,QString name,QString label);
    QString _indihost;
    int _indiport;
    QStringList _availableModuleLibs;

};
#endif
