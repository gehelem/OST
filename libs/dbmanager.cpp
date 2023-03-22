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
    QFile file;
    file.setFileName(":db.sql");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList queries = QTextStream(&file).readAll().split(";");
    file.close();
    foreach (QString sql, queries)
    {
        if (sql.trimmed().isEmpty())
        {
            continue;
        }
        else
        {
            if (!mQuery.exec(sql))
            {
                sendError("CreateDatabaseStructure ERROR SQL =" + sql);
                sendError("CreateDatabaseStructure - ERROR : " + mQuery.lastError().text());
            }
        }

    }


}
bool DBManager::setDbProfile(const QString &pModuleType, const QString &pProfileName, QVariantMap &profile )
{
    if(!mDb.open())
    {
        sendError("setDbProfile dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
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
        sendError("getDbProfile dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "SELECT ALLVALUES FROM PROFILES WHERE MODULETYPE='" + pModuleType + "' AND PROFILENAME='" + pProfileName +
                  "'";
    if (!mQuery.exec(sql))
    {
        sendError("getProfile - ERROR SQL =" + sql);
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
        sendError("getDbProfiles dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "SELECT PROFILENAME,ALLVALUES FROM PROFILES WHERE MODULETYPE='" + moduleType + "' ";
    if (!mQuery.exec(sql))
    {
        sendError("getProfiles - ERROR SQL =" + sql);
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
        sendError("getDbConfiguration dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='" + configName + "'";
    if (!mQuery.exec(sql))
    {
        sendError("getConfiguration - ERROR SQL =" + sql);
        sendError("getConfiguration - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QVariantMap line;
        line["moduletype"] = mQuery.value(1).toString().toUtf8();
        line["profilename"] = mQuery.value(2).toString().toUtf8();
        result[mQuery.value(0).toString().toUtf8()] = line;
    }
    mDb.close();
    return true;
}
bool DBManager::getDbConfigurations(QVariantMap &result )
{
    if(!mDb.open())
    {
        sendError("getDbConfigurations dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "SELECT CONFIGNAME,MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS";
    if (!mQuery.exec(sql))
    {
        sendError("getDbConfigurations - ERROR SQL =" + sql);
        sendError("getDbConfigurations - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QVariantMap line;
        line["modulename"] = mQuery.value(1).toString().toUtf8();
        line["moduletype"] = mQuery.value(2).toString().toUtf8();
        line["profilename"] = mQuery.value(3).toString().toUtf8();
        result[mQuery.value(0).toString().toUtf8()] = line;
    }
    mDb.close();
    return true;
}
