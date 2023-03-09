#ifndef DBMANAGER_h_
#define DBMANAGER_h_

#include "baseroot.h"
#include <QtCore>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QObject>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <boost/log/trivial.hpp>


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

    private:
        void CreateDatabaseStructure();
        QString mDbPath;
        QSqlDatabase mDb;
        QSqlQuery mQuery;
};


#endif // DBMANAGER_H
