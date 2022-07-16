#ifndef CONTROLLER_h_
#define CONTROLLER_h_
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
//#include <QtSql/QSqlError>
#include <boost/log/trivial.hpp>

#include <basemodule.h>
#include "utils/propertyvisitor.h"
#include "utils/propertyfactory.h"
#include "utils/propertytextdumper.h"

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
    void OnPropertyCreated(Property *pProperty, QString *pModulename);
    void OnPropertyUpdated(Property *pProperty, QString *pModulename);
    void OnPropertyAppended(Property *pProperty, QString *pModulename);
    void OnPropertyRemoved(Property *pProperty, QString *pModulename);
    void OnNewMessageSent(QString message, QString *pModulename, QString Device);
    void OnModuleDumped(QMap<QString, QMap<QString, QMap<QString, Property*>>> treeList, QString* pModulename, QString* pModulelabel);
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
