#ifndef DBMANAGER_h_
#define DBMANAGER_h_

#include "baseroot.h"

class DBManager : virtual public Baseroot
{
    public:
        DBManager();
        ~DBManager();
        bool dbInit(const QString &pDbPath, const QString &pConnectionName);
        bool getDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &result );
        bool setDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile );
        bool getDbProfiles(QString moduleType, QVariantMap &result );
        bool getDbConfiguration(QString configName, QVariantMap &result );
        bool getDbConfigurations(QVariantMap &result );

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
