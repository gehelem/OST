#ifndef BASEROOT_h_
#define BASEROOT_h_
#include "translate.h"
#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class Baseroot : public QObject
{
    public:
        Baseroot();
        ~Baseroot();
        QVariantMap getMessages(void);
        QVariantMap getErrors(void);
        QVariantMap getWarnings(void);
        void resetMessages(void);
        void resetErrors(void);
        void resetWarnings(void);
        void setMessagesSize (int &size);
        void setErrorsSize (int &size);
        void setWarningsSize (int &size);
        void sendMessage(const QString &pMessage);
        void sendError(const QString &pMessage);
        void sendWarning(const QString &pMessage);
        void sendConsole(const QString &pMessage);
        virtual void OnModuleEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                   const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType);
            Q_UNUSED(pEventModule);
            Q_UNUSED(pEventKey);
            Q_UNUSED(pEventData);
        };
        void setLng(const QString &pLng);
    protected:
    private:
        QList<QVariantMap> mMessages;
        QList<QVariantMap> mErrors;
        QList<QVariantMap> mWarnings;
        int mMessagesSize = 20;
        int mErrorsSize = 20;
        int mWarningsSize = 20;

}
;
#endif

