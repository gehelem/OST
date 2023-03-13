#ifndef BASEROOT_h_
#define BASEROOT_h_
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

class Baseroot
{
    public:
        Baseroot();
        ~Baseroot();
        QVariantMap getMessages(void)
        {
            QVariantMap map;
            for( int i = 0; i < mMessages.count(); ++i )
            {
                map[QString::number(i)] = mMessages[i];
            }

            return map;
        }
        QVariantMap getErrors(void)
        {
            QVariantMap map;
            for( int i = 0; i < mErrors.count(); ++i )
            {
                map[QString::number(i)] = mErrors[i];
            }

            return map;
        }
        QVariantMap getWarnings(void)
        {
            QVariantMap map;
            for( int i = 0; i < mWarnings.count(); ++i )
            {
                map[QString::number(i)] = mWarnings[i];
            }

            return map;
        }
        void resetMessages(void)
        {
            mMessages.clear();
        }
        void resetErrors(void)
        {
            mErrors.clear();
        }
        void resetWarnings(void)
        {
            mWarnings.clear();
        }
        void setMessagesSize (int &size)
        {
            mMessagesSize = size;
        }
        void setErrorsSize (int &size)
        {
            mErrorsSize = size;
        }
        void setWarningsSize (int &size)
        {
            mWarningsSize = size;
        }
    protected:
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

