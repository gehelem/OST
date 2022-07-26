#include <QtCore>
#include <boost/log/trivial.hpp>
#include "dbmanager.h"

DBManager::DBManager(QObject *parent, const QString &dbpath) :_dbpath(dbpath)
{
    BOOST_LOG_TRIVIAL(debug) << "DBManager";
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        _db = QSqlDatabase::addDatabase(DRIVER);
        _db.setDatabaseName(_dbpath+"ost.db" );
        _query = QSqlQuery(_db);
        if(!_db.open()) {
            qDebug() << "dbOpen - ERROR: " << _db.databaseName() << " - " << _db.lastError().text().toLocal8Bit().data();
        } else {
            QFile _file;
            _file.setFileName(":db.sql");
            _file.open(QIODevice::ReadOnly | QIODevice::Text);
            QStringList _queries = QTextStream(&_file).readAll().split(";");
            _file.close();
            foreach (QString _sql, _queries) {
                BOOST_LOG_TRIVIAL(debug) <<  "SQL :" << _sql.toStdString();
                if (_sql.trimmed().isEmpty()) continue;
                if (!_query.exec(_sql)) qDebug() << "DatabaseQuery - ERROR - " << _query.lastError().text().toLocal8Bit().data();

            }


        }
    }
    else
        qDebug() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";
}


DBManager::~DBManager()
{

}
