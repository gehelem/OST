#include "lovjsondumper.h"
#include <sstream>
namespace  OST
{

QJsonObject LovJsonDumper::dumpLovCommons(LovBase *pLov)
{
    QJsonObject json;

    json["label"] = pLov->label();
    json["type"] = pLov->getType();

    return json;

}
void LovJsonDumper::visit(LovString *pLov)
{
    QJsonObject json = dumpLovCommons(pLov);
    if (pLov->getLov().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const QString &key, pLov->getLov().keys())
        {
            lines[key] = pLov->getLov()[key];
        }
        json["values"] = lines;
    }
    mResult = json;
}
void LovJsonDumper::visit(LovInt *pLov)
{
    QJsonObject json = dumpLovCommons(pLov);
    if (pLov->getLov().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const long &key, pLov->getLov().keys())
        {
            QString skey = QString::number(key);
            lines[skey] = pLov->getLov()[key];
        }
        json["values"] = lines;
    }
    mResult = json;
}
void LovJsonDumper::visit(LovFloat *pLov)
{
    QJsonObject json = dumpLovCommons(pLov);
    if (pLov->getLov().size() > 0)
    {
        QJsonObject lines = QJsonObject();
        foreach(const double &key, pLov->getLov().keys())
        {
            QString skey = QString::number(key);
            lines[skey] = pLov->getLov()[key];
        }
        json["values"] = lines;
    }
    mResult = json;
}

}
