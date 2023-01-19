#include <QtCore>
#include <boost/log/trivial.hpp>
#include "dbmanager.h"

DBManager::DBManager(QObject *parent, const QString &dbpath) :_dbpath(dbpath)
{
    if(QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        bool _dbExists = QFile::exists(_dbpath+"ost.db");
        _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(_dbpath+"ost.db" );
        _query = QSqlQuery(_db);
        if(!_db.open()) {
            BOOST_LOG_TRIVIAL(debug) << "dbOpen - ERROR: " << _db.databaseName().toStdString() << " - " << _db.lastError().text().toStdString();
        } else {
            if (!_dbExists) CreateDatabaseStructure();
        }
    } else {
        BOOST_LOG_TRIVIAL(debug)  << "DatabaseConnect - ERROR: QSQLITE driver unavailable";
    }

}


DBManager::~DBManager()
{

}


void DBManager::CreateDatabaseStructure()
{
    qDebug() << "OST database creation with default values";
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
bool DBManager::setProfile(QString moduleType,QString profileName, QVariantMap &profile )
{
    QJsonObject  obj=QJsonObject::fromVariantMap(profile);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    QString sql="INSERT OR REPLACE INTO PROFILES ('MODULETYPE','PROFILENAME','ALLVALUES') VALUES ('"+moduleType+"','"+profileName+"','"+strJson+"');";
    if (!_query.exec(sql)) {
        BOOST_LOG_TRIVIAL(debug) << "setProfile - ERROR SQL =" << sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "setProfile - ERROR : " << _query.lastError().text().toLocal8Bit().data();
        return false;
    }
    return true;
}
bool DBManager::getProfile(QString moduleType,QString profileName, QVariantMap &result )
{
    QString _sql = "SELECT ALLVALUES FROM PROFILES WHERE MODULETYPE='"+moduleType+"' AND PROFILENAME='"+profileName+"'";
    if (!_query.exec(_sql)) {
        BOOST_LOG_TRIVIAL(debug) << "getProfile - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getProfile - ERROR : " << _query.lastError().text().toLocal8Bit().data();
    }
    while (_query.next()) {
        QJsonDocument res = QJsonDocument::fromJson(_query.value(0).toString().toUtf8());
        QJsonObject  obj = res.object();
        result = obj.toVariantMap();
        return true;
    }
    return false;
}
bool DBManager::getProfiles(QString moduleType, QVariantMap &result )
{
    QString _sql = "SELECT PROFILENAME,ALLVALUES FROM PROFILES WHERE MODULETYPE='"+moduleType+"' ";
    if (!_query.exec(_sql)) {
        BOOST_LOG_TRIVIAL(debug) << "getProfiles - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getProfiles - ERROR : " << _query.lastError().text().toLocal8Bit().data();
    }
    while (_query.next()) {
        QString name=_query.value(0).toString().toUtf8();
        QJsonDocument res = QJsonDocument::fromJson(_query.value(1).toString().toUtf8());
        QJsonObject  obj = res.object();
        QVariantMap line = obj.toVariantMap();
        result [name]=line;
    }
    return true;
}
bool DBManager::getConfiguration(QString configName, QVariantMap &result )
{
    QString _sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='"+configName+"'";
    if (!_query.exec(_sql)) {
        BOOST_LOG_TRIVIAL(debug) << "getConfiguration - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getConfiguration - ERROR : " << _query.lastError().text().toLocal8Bit().data();
    }
    while (_query.next()) {
        QVariantMap _line;
        _line["moduletype"]=_query.value(1).toString().toUtf8();
        _line["profilename"]=_query.value(2).toString().toUtf8();
        result[_query.value(0).toString().toUtf8()]=_line;
    }
    return true;
}
