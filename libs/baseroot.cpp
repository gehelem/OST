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
    QDateTime dt = QDateTime::currentDateTime();
    QVariantMap m;
    m["message"] = pMessage;
    m["datetime"] = dt;
    mMessages.append(m);
    if (mMessages.size() > mMessagesSize) mMessages.removeFirst();
    OnModuleEvent("mm", QString(), QString(), m);
}
void Baseroot::sendError(const QString &pMessage)
{
    QDateTime dt = QDateTime::currentDateTime();
    QVariantMap m;
    m["error"] = pMessage;
    m["datetime"] = dt;
    mErrors.append(m);
    if (mErrors.size() > mErrorsSize) mErrors.removeFirst();
    OnModuleEvent("me", QString(), QString(), m);
}
void Baseroot::sendWarning(const QString &pMessage)
{
    QDateTime dt = QDateTime::currentDateTime();
    QVariantMap m;
    m["warning"] = pMessage;
    m["datetime"] = dt;
    mWarnings.append(m);
    if (mWarnings.size() > mWarningsSize) mWarnings.removeFirst();
    OnModuleEvent("mw", QString(), QString(), m);
}
