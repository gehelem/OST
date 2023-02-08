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
    OnModuleEvent("mm", QString(), QString(), m);
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}

