#ifndef DBMANAGER_h_
#define DBMANAGER_h_

#include <QtCore>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QObject>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <boost/log/trivial.hpp>


class DBManager : public QObject
{
        Q_OBJECT
    public:
        DBManager(QObject *parent, const QString &dbpath);
        ~DBManager();
        bool getProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &result );
        bool setProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile );
        bool getProfiles(QString moduleType, QVariantMap &result );
        bool getConfiguration(QString configName, QVariantMap &result );

    private:
        void CreateDatabaseStructure();
        QString mDbPath;
        QSqlDatabase mDb;
        QSqlQuery mQuery;
};


#endif // DBMANAGER_H
