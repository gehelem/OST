#include <QtCore>
#include <boost/log/trivial.hpp>
#include "dbmanager.h"

DBManager::DBManager(QObject *parent, const QString &dbpath) :_dbpath(dbpath)
{
    if(QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(_dbpath+"ost.db" );
        _query = QSqlQuery(_db);
        if(!_db.open()) {
            BOOST_LOG_TRIVIAL(debug) << "dbOpen - ERROR: " << _db.databaseName().toStdString() << " - " << _db.lastError().text().toStdString();
        } else {
            CreateDatabaseStructure();
        }
    }
    else
        BOOST_LOG_TRIVIAL(debug)  << "DatabaseConnect - ERROR: QSQLITE driver unavailable";
}


DBManager::~DBManager()
{

}


void DBManager::CreateDatabaseStructure()
{
    QFile _file;
    _file.setFileName(":db.sql");
    _file.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList _queries = QTextStream(&_file).readAll().split(";");
    _file.close();
    foreach (QString _sql, _queries) {
        if (_sql.trimmed().isEmpty()) {
            continue;
        } else {
            if (!_query.exec(_sql)) {
                BOOST_LOG_TRIVIAL(debug) <<  "CreateDatabaseStructure ERROR SQL =" << _sql.toStdString();
                BOOST_LOG_TRIVIAL(debug) << "CreateDatabaseStructure - ERROR : " << _query.lastError().text().toLocal8Bit().data();
            }
        }

    }

}
