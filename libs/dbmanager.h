#ifndef DBMANAGER_h_
#define DBMANAGER_h_

#include "datastore.h"

struct catalogResult
{
    QString  catalog;
    QString  code;
    double RA;
    QString  NS;
    double DEC;
    double diam;
    double mag;
    QString  name;
    QString  alias;

};


class DBManager : virtual public Datastore
{
    public:
        DBManager();
        ~DBManager();
        bool dbInit(const QString &pDbPath, const QString &pConnectionName);
        bool getDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &result );
        bool setDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile );
        bool getDbProfiles(QString moduleType, QVariantMap &result );
        bool getDbConfiguration(const QString &pConfigName, QVariantMap &result );
        bool saveDbConfiguration(const QString &pConfigName, QMap<QString, QMap<QString, QString>> &pConf);
        bool getDbConfigurations(QVariantMap &result );
        bool searchCatalog(const QString &pArgument, QList<catalogResult> &pResult);
        bool populateCatalog(const QString &pFileName, const QString &pName);

    private:
        void CreateDatabaseStructure();
        QString mDbPath;
        QSqlDatabase mDb;
        QSqlQuery mQuery;
    signals:
        void dbEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                     const QVariantMap &pEventData);

};


#endif // DBMANAGER_H
