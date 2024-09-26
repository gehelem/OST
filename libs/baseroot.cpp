#include <QtCore>
#include "baseroot.h"

Baseroot::Baseroot()
{
}
Baseroot::~Baseroot()
{
}
QVariantMap Baseroot::getMessages(void)
{
    QVariantMap map;
    for( int i = 0; i < mMessages.count(); ++i )
    {
        map[QString::number(i)] = mMessages[i];
    }

    return map;
}
QVariantMap Baseroot::getErrors(void)
{
    QVariantMap map;
    for( int i = 0; i < mErrors.count(); ++i )
    {
        map[QString::number(i)] = mErrors[i];
    }

    return map;
}
QVariantMap Baseroot::getWarnings(void)
{
    QVariantMap map;
    for( int i = 0; i < mWarnings.count(); ++i )
    {
        map[QString::number(i)] = mWarnings[i];
    }

    return map;
}
void Baseroot::resetMessages(void)
{
    mMessages.clear();
}
void Baseroot::resetErrors(void)
{
    mErrors.clear();
}
void Baseroot::resetWarnings(void)
{
    mWarnings.clear();
}
void Baseroot::setMessagesSize (int &size)
{
    mMessagesSize = size;
}
void Baseroot::setErrorsSize (int &size)
{
    mErrorsSize = size;
}
void Baseroot::setWarningsSize (int &size)
{
    mWarningsSize = size;
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
    sendConsole("info " + pMessage);
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
    sendConsole("error " + pMessage);
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
    sendConsole("warning " + pMessage);
}
void Baseroot::sendConsole(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}
void Baseroot::setLng(const QString &pLng)
{
    //mTranslate->SetLanguage(pLng);
}
