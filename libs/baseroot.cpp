#include <QtCore>
#include "baseroot.h"

Baseroot::Baseroot()
{
}
Baseroot::~Baseroot()
{
}
void Baseroot::sendMessage(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QVariantMap m;
    m["message"] = messageWithDateTime;
    m["type"] = "info";
    OnModuleEvent("mm", QString(), QString(), m);
}

void Baseroot::sendWarning(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QVariantMap m;
    m["message"] = messageWithDateTime;
    m["type"] = "warning";
    OnModuleEvent("mm", QString(), QString(), m);
}
void Baseroot::sendError(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QVariantMap m;
    m["message"] = messageWithDateTime;
    m["type"] = "error";
    OnModuleEvent("mm", QString(), QString(), m);
}
