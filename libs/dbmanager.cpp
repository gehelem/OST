#include <QtCore>
#include <boost/log/trivial.hpp>
#include "dbmanager.h"

DBManager::DBManager()
{

}


DBManager::~DBManager()
{

}

bool DBManager::dbInit(const QString &pDbPath, const QString &pConnectionName)
{
    mDbPath = pDbPath;
    if(QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        bool mDbExists = QFile::exists(mDbPath + "ost.db");
        mDb = QSqlDatabase::addDatabase("QSQLITE", pConnectionName);
        mDb.setDatabaseName(mDbPath + "ost.db" );
        mQuery = QSqlQuery(mDb);

        if(!mDb.open())
        {
            sendMessage("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
            return false;
        }
        if (!mDbExists) CreateDatabaseStructure();
        return true;
    }
    else
    {
        sendMessage("DatabaseConnect - ERROR: QSQLITE driver unavailable");
        return false;
    }
    return false;
}


void DBManager::CreateDatabaseStructure()
{
    qDebug() << "OST database creation with default values";
    QFile _file;
    _file.setFileName(":db.sql");
    _file.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList _queries = QTextStream(&_file).readAll().split(";");
    _file.close();
    foreach (QString _sql, _queries)
    {
        if (_sql.trimmed().isEmpty())
        {
            continue;
        }
        else
        {
            if (!mQuery.exec(_sql))
            {
                BOOST_LOG_TRIVIAL(debug) <<  "CreateDatabaseStructure ERROR SQL =" << _sql.toStdString();
                BOOST_LOG_TRIVIAL(debug) << "CreateDatabaseStructure - ERROR : " << mQuery.lastError().text().toLocal8Bit().data();
            }
        }

    }

}
bool DBManager::setDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile )
{
    QJsonObject  obj = QJsonObject::fromVariantMap(profile);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    QString sql = "INSERT OR REPLACE INTO PROFILES ('MODULETYPE','PROFILENAME','ALLVALUES') VALUES ('" + pModuleType + "','" +
                  pProfileName + "','" + strJson + "');";
    if (!mQuery.exec(sql))
    {
        BOOST_LOG_TRIVIAL(debug) << "setProfile - ERROR SQL =" << sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "setProfile - ERROR : " << mQuery.lastError().text().toLocal8Bit().data();
        return false;
    }
    return true;
}
bool DBManager::getDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &result )
{
    QString _sql = "SELECT ALLVALUES FROM PROFILES WHERE MODULETYPE='" + pModuleType + "' AND PROFILENAME='" + pProfileName +
                   "'";
    if (!mQuery.exec(_sql))
    {
        BOOST_LOG_TRIVIAL(debug) << "getProfile - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getProfile - ERROR : " << mQuery.lastError().text().toLocal8Bit().data();
    }
    while (mQuery.next())
    {
        QJsonDocument res = QJsonDocument::fromJson(mQuery.value(0).toString().toUtf8());
        QJsonObject  obj = res.object();
        result = obj.toVariantMap();
        return true;
    }
    return false;
}
bool DBManager::getDbProfiles(QString moduleType, QVariantMap &result )
{
    QString _sql = "SELECT PROFILENAME,ALLVALUES FROM PROFILES WHERE MODULETYPE='" + moduleType + "' ";
    if (!mQuery.exec(_sql))
    {
        BOOST_LOG_TRIVIAL(debug) << "getProfiles - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getProfiles - ERROR : " << mQuery.lastError().text().toLocal8Bit().data();
    }
    while (mQuery.next())
    {
        QString name = mQuery.value(0).toString().toUtf8();
        QJsonDocument res = QJsonDocument::fromJson(mQuery.value(1).toString().toUtf8());
        QJsonObject  obj = res.object();
        QVariantMap line = obj.toVariantMap();
        result [name] = line;
    }
    return true;
}
bool DBManager::getDbConfiguration(QString configName, QVariantMap &result )
{
    QString _sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='" + configName + "'";
    if (!mQuery.exec(_sql))
    {
        BOOST_LOG_TRIVIAL(debug) << "getConfiguration - ERROR SQL =" << _sql.toStdString();
        BOOST_LOG_TRIVIAL(debug) << "getConfiguration - ERROR : " << mQuery.lastError().text().toLocal8Bit().data();
    }
    while (mQuery.next())
    {
        QVariantMap _line;
        _line["moduletype"] = mQuery.value(1).toString().toUtf8();
        _line["profilename"] = mQuery.value(2).toString().toUtf8();
        result[mQuery.value(0).toString().toUtf8()] = _line;
    }
    return true;
}
