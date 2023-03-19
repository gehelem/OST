#include <QtCore>
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
            sendError("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
            return false;
        }
        if (!mDbExists) CreateDatabaseStructure();
        mDb.close();
        return true;
    }
    else
    {
        sendError("DatabaseConnect - ERROR: QSQLITE driver unavailable");
        return false;
    }
    return false;
}


void DBManager::CreateDatabaseStructure()
{

    sendMessage("OST database creation with default values");
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
                sendError("CreateDatabaseStructure ERROR SQL =" + _sql);
                sendError("CreateDatabaseStructure - ERROR : " + mQuery.lastError().text());
            }
        }

    }


}
bool DBManager::setDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile )
{
    if(!mDb.open())
    {
        sendError("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }

    QJsonObject  obj = QJsonObject::fromVariantMap(profile);
    QJsonDocument doc(obj);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    QString sql = "INSERT OR REPLACE INTO PROFILES ('MODULETYPE','PROFILENAME','ALLVALUES') VALUES ('" + pModuleType + "','" +
                  pProfileName + "','" + strJson + "');";
    if (!mQuery.exec(sql))
    {
        sendError("setProfile - ERROR SQL =" + sql);
        sendError("setProfile - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    mDb.close();
    return true;

}
bool DBManager::getDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &result )
{
    if(!mDb.open())
    {
        sendError("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString _sql = "SELECT ALLVALUES FROM PROFILES WHERE MODULETYPE='" + pModuleType + "' AND PROFILENAME='" + pProfileName +
                   "'";
    if (!mQuery.exec(_sql))
    {
        sendError("getProfile - ERROR SQL =" + _sql);
        sendError("getProfile - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QJsonDocument res = QJsonDocument::fromJson(mQuery.value(0).toString().toUtf8());
        QJsonObject  obj = res.object();
        result = obj.toVariantMap();
        mDb.close();
        return true;
    }
    mDb.close();
    return false;
}
bool DBManager::getDbProfiles(QString moduleType, QVariantMap &result )
{
    if(!mDb.open())
    {
        sendError("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString _sql = "SELECT PROFILENAME,ALLVALUES FROM PROFILES WHERE MODULETYPE='" + moduleType + "' ";
    if (!mQuery.exec(_sql))
    {
        sendError("getProfiles - ERROR SQL =" + _sql);
        sendError("getProfiles - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QString name = mQuery.value(0).toString().toUtf8();
        QJsonDocument res = QJsonDocument::fromJson(mQuery.value(1).toString().toUtf8());
        QJsonObject  obj = res.object();
        QVariantMap line = obj.toVariantMap();
        result [name] = line;
    }
    mDb.close();
    return true;
}
bool DBManager::getDbConfiguration(QString configName, QVariantMap &result )
{
    if(!mDb.open())
    {
        sendError("dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString _sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='" + configName + "'";
    if (!mQuery.exec(_sql))
    {
        sendError("getConfiguration - ERROR SQL =" + _sql);
        sendError("getConfiguration - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QVariantMap _line;
        _line["moduletype"] = mQuery.value(1).toString().toUtf8();
        _line["profilename"] = mQuery.value(2).toString().toUtf8();
        result[mQuery.value(0).toString().toUtf8()] = _line;
    }
    mDb.close();
    return true;
}
