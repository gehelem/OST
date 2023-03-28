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
bool DBManager::getDbConfiguration(const QString &pConfigName, QVariantMap &result )
{
    if(!mDb.open())
    {
        sendError("getDbConfiguration dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "SELECT MODULENAME,MODULETYPE,PROFILENAME FROM CONFIGURATIONS WHERE CONFIGNAME='" + pConfigName + "'";
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
bool DBManager::saveDbConfiguration(const QString &pConfigName, QMap<QString, QMap<QString, QString> > &pConf)
{
    if(!mDb.open())
    {
        sendError("setDbConfiguration dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    foreach(const QString &key, pConf.keys())
    {
        QString label = pConf[key]["label"];
        QString type = pConf[key]["type"];
        QString profile = pConf[key]["profile"];
        qDebug() << "DB conf = " << key << " label = " << label << " type = " << type << " profile = " << profile;
        QString sql = "INSERT OR REPLACE INTO CONFIGURATIONS (CONFIGNAME,MODULENAME,MODULETYPE,PROFILENAME) VALUES ('" + pConfigName
                      + "','" + label + "','" + type + "','" + profile + "');";
        if (!mQuery.exec(sql))
        {
            sendError("setDbConfiguration - ERROR SQL =" + sql);
            sendError("setDbConfiguration - ERROR : " + mQuery.lastError().text());
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
bool DBManager::searchCatalog(const QString &pArgument, QList<catalogResult> &pResult)
{
    if(!mDb.open())
    {
        sendError("searchCatalog dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
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
        sendError("searchCatalog - ERROR SQL =" + sql);
        sendError("searchCatalog - ERROR : " + mQuery.lastError().text());
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
        sendError("populateCatalog dbOpen - ERROR: " + mDb.databaseName() + " - " + mDb.lastError().text());
        return false;
    }
    QString sql = "DELETE FROM CATALOGS WHERE CATALOG='" + pName + "'";
    if (!mQuery.exec(sql))
    {
        sendError("searchCatalog - ERROR SQL =" + sql);
        sendError("searchCatalog - ERROR : " + mQuery.lastError().text());
        mDb.close();
        return false;
    }


    QFile inputFile(pFileName);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        sendError("populateCatalog can't read file " + pFileName);
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
            sendWarning("populateCatalog with file " + pFileName + " ; can't parse this line : " + line);
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
                sendWarning("searchCatalog - ERROR SQL =" + sql);
                sendWarning("searchCatalog - ERROR : " + mQuery.lastError().text());
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
