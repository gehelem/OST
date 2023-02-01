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
    QVariantMap m;
    m["message"] = pMessage;
    OnModuleEvent("message", QString(), QString(), m);
}

