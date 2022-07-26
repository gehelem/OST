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
    DBManager(QObject *parent,const QString &dbpath);
    ~DBManager();
    void CreateDatabaseStructure();

private:
    QString _dbpath;
    QSqlDatabase _db;
    QSqlQuery _query;
};


#endif // DBMANAGER_H
