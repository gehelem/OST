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
            logError("dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
            return false;
        }
        if (!mDbExists) CreateDatabaseStructure();
        mDb.close();
        return true;
    }
    else
    {
        logError("DatabaseConnect - ERROR: QSQLITE driver unavailable");
        return false;
    }
    return false;
}


void DBManager::CreateDatabaseStructure()
{

    logInfo("OST database creation with default values");
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
                logError("CreateDatabaseStructure ERROR SQL = %1", {sql});
                logError("CreateDatabaseStructure - ERROR : %1", {mQuery.lastError().text()});
            }
        }

    }
    // add Messier and common stars
    populateCatalog(":messier.txt", "Messier");
    populateCatalog(":stars.txt", "Stars");

}
bool DBManager::setDbProfile(const QString &pModuleType, const QString &pProfileName, QJsonObject &content )
{
    if(!mDb.open())
    {
        logError("setDbProfile dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }

    QJsonDocument doc(content);
    QByteArray docByteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson = QLatin1String(docByteArray);
    QString sql = "INSERT OR REPLACE INTO PROFILES ('MODULETYPE','PROFILENAME','ALLVALUES') VALUES ('" + pModuleType + "','" +
                  pProfileName + "','" + strJson + "');";
    if (!mQuery.exec(sql))
    {
        logError("setProfile - ERROR SQL = %1", {sql});
        logError("setProfile - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }
    mDb.close();
    return true;

}
bool DBManager::getDbProfile(const QString &pModuleType, const QString &pProfileName, QJsonObject &content )
{
    if(!mDb.open())
    {
        logError("getDbProfile dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "SELECT ALLVALUES FROM PROFILES WHERE MODULETYPE='" + pModuleType + "' AND PROFILENAME='" + pProfileName +
                  "'";
    if (!mQuery.exec(sql))
    {
        logError("getDbProfile - ERROR SQL = %1", {sql});
        logError("getDbProfile - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QJsonDocument res = QJsonDocument::fromJson(mQuery.value(0).toString().toUtf8());
        content = res.object();
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
        logError("getDbProfiles dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "SELECT PROFILENAME,ALLVALUES FROM PROFILES WHERE MODULETYPE='" + moduleType + "' ";
    if (!mQuery.exec(sql))
    {
        logError("getDbProfiles - ERROR SQL = %1", {sql});
        logError("getDbProfiles - ERROR : %1", {mQuery.lastError().text()});
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
bool DBManager::getDbProfiles(QVariantMap &result )
{
    if(!mDb.open())
    {
        logError("getDbProfiles dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "SELECT MODULETYPE,PROFILENAME FROM PROFILES ";
    if (!mQuery.exec(sql))
    {
        logError("getDbProfiles - ERROR SQL = %1", {sql});
        logError("getDbProfiles - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        QString type = mQuery.value(0).toString().toUtf8();
        QString name = mQuery.value(1).toString().toUtf8();
        QVariantList l = result[type].toList();
        l.append(name);
        result[type] = l;
    }
    mDb.close();
    return true;
}
bool DBManager::getDbConfiguration(const QString &pConfigName, QVariantMap &result )
{
    if(!mDb.open())
    {
        logError("getDbConfiguration dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='" + pConfigName + "'";
    if (!mQuery.exec(sql))
    {
        logError("getDbConfiguration - ERROR SQL = %1", {sql});
        logError("getDbConfiguration - ERROR : %1", {mQuery.lastError().text()});
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
bool DBManager::saveDbConfiguration(const QString &pConfigName, QVariantMap &pConf)
{
    if(!mDb.open())
    {
        logError("saveDbConfiguration dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }

    QString sql = "DELETE FROM CONFIGURATIONS WHERE CONFIGNAME= '" + pConfigName + "';";
    if (!mQuery.exec(sql))
    {
        logError("saveDbConfiguration - ERROR SQL = %1", {sql});
        logError("saveDbConfiguration - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }

    foreach(const QString &key, pConf.keys())
    {
        QString label = pConf[key].toMap()["label"].toString();
        QString type = pConf[key].toMap()["type"].toString();
        QString profile = pConf[key].toMap()["profile"].toString();
        QString sql = "INSERT OR REPLACE INTO CONFIGURATIONS (CONFIGNAME,MODULENAME,MODULETYPE,PROFILENAME) VALUES ('" + pConfigName
                      + "','" + label + "','" + type + "','" + profile + "');";
        if (!mQuery.exec(sql))
        {
            logError("saveDbConfiguration - ERROR SQL = %1", {sql});
            logError("saveDbConfiguration - ERROR : %1", {mQuery.lastError().text()});
            mDb.close();
            return false;
        }

    }
    mDb.close();
    return true;


}
bool DBManager::getDbConfigurations(QVariantMap &result )
{
    if(!mDb.open())
    {
        logError("getDbConfigurations dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "SELECT CONFIGNAME,MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS";
    if (!mQuery.exec(sql))
    {
        logError("getDbConfigurations - ERROR SQL = %1", {sql});
        logError("getDbConfigurations - ERROR : %1", {mQuery.lastError().text()});
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
bool DBManager::searchCatalog(const QString &pArgument, QList<catalogResult> &pResult)
{
    if(!mDb.open())
    {
        logError("searchCatalog dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString argumentWithoutBlanks = pArgument;
    argumentWithoutBlanks.replace(" ", "");
    QString sql = "SELECT CATALOG,CODE,RA,NS,DEC,DIAM,MAG,NAME,ALIAS FROM CATALOGS WHERE ";
    sql = sql + "    UPPER(CODE) LIKE UPPER('%" + argumentWithoutBlanks + "%')";
    sql = sql + " OR UPPER(ALIAS) LIKE UPPER('%" + argumentWithoutBlanks + "%')";
    sql = sql + " OR UPPER(NAME) LIKE UPPER('%" + argumentWithoutBlanks + "%')";
    if (!mQuery.exec(sql))
    {
        logError("searchCatalog - ERROR SQL = %1", {sql});
        logError("searchCatalog - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }
    while (mQuery.next())
    {
        catalogResult line;
        line.catalog = mQuery.value(0).toString();
        line.code = mQuery.value(1).toString();
        line.RA = mQuery.value(2).toDouble();
        line.NS = mQuery.value(3).toString();
        line.DEC = mQuery.value(4).toDouble();
        line.diam = mQuery.value(5).toDouble();
        line.mag = mQuery.value(6).toDouble();
        line.name = mQuery.value(7).toString();
        line.alias = mQuery.value(8).toString();
        pResult.push_back(line);
    }
    mDb.close();
    return true;

}
bool DBManager::populateCatalog(const QString &pFileName, const QString &pName)
{
    if(!mDb.open())
    {
        logError("populateCatalog dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return false;
    }
    QString sql = "DELETE FROM CATALOGS WHERE CATALOG='" + pName + "'";
    if (!mQuery.exec(sql))
    {
        logError("searchCatalog - ERROR SQL = %1", {sql});
        logError("searchCatalog - ERROR : %1", {mQuery.lastError().text()});
        mDb.close();
        return false;
    }


    QFile inputFile(pFileName);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        logError("populateCatalog can't read file %1", {pFileName});
        return false;
    }
    QTextStream in(&inputFile);
    while (!in.atEnd())
    {

        QString line = in.readLine();
        line = line.replace("'", " ");
        QStringList splitted = line.split(u';');
        if (splitted.size() >= 10)
        {
            logWarning("populateCatalog with file %1 ; can't parse this line : %2", {pFileName, line});
        }
        else
        {
            sql = "INSERT INTO CATALOGS VALUES (";
            sql = sql + "'" + pName + "'";
            sql = sql + ",'" + splitted[0] + "'";
            sql = sql + ",'" + splitted[1] + "'";
            sql = sql + ",'" + splitted[2] + "'";
            sql = sql + ",'" + splitted[3] + "'";
            if (splitted.size() > 4) sql = sql + ",'" + splitted[4] + "'";
            else  sql = sql + ",''";
            if (splitted.size() > 5) sql = sql + ",'" + splitted[5] + "'";
            else  sql = sql + ",''";
            if (splitted.size() > 6) sql = sql + ",'" + splitted[6] + "'";
            else  sql = sql + ",''";
            if (splitted.size() > 7) sql = sql + ",'" + splitted[7] + "'";
            else  sql = sql + ",''";
            sql = sql + ");";
            if (!mQuery.exec(sql))
            {
                logWarning("searchCatalog - ERROR SQL = %1", {sql});
                logWarning("searchCatalog - ERROR : %1", {mQuery.lastError().text()});
            }

        }
    }
    mDb.close();
    inputFile.close();
    return true;
    /*QFile file;
    file.setFileName(pFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    content = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString &key, props.keys())
    {
        QVariant tt = props[key].toVariant();
        mProperties[key] = tt.toMap();
    }*/


}
QString DBManager::getGrants(const QString &pUser, const QString &pPW)
{
    QString result = "-1";

    if(!mDb.open())
    {
        logError("getGrants dbOpen - ERROR: %1 - %2", {mDb.databaseName(), mDb.lastError().text()});
        return result;
    }
    QString sql = "SELECT GRANT FROM USERS WHERE ";
    sql = sql + "    LOGIN = '" + pUser + "'";
    sql = sql + " AND PW= '" + pPW + "'";
    if (!mQuery.exec(sql))
    {
        logError("getGrants - %1", {mQuery.lastError().text()});
        mDb.close();
        return result;
    }
    if (mQuery.size() > 1)
    {
        mDb.close();
        logError("getGrants - Returns more than one row - %1", {pUser});
        return result;
    }
    if (mQuery.size() == 0)
    {
        mDb.close();
        logError("getGrants - Invalid credentials - %1", {pUser});
        return result;
    }
    mQuery.next();
    // 0 : user is read only
    // 1 : user has full access
    // -1 : invalid
    result = mQuery.value(0).toString();
    if ((result != "0") && (result != "1"))
    {
        logError("getGrants - Invalid credentials - %1-%2", {pUser, result});
        result = "-1";
        return result;
    }
    mDb.close();
    return result;

}
