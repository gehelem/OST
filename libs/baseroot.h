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

namespace OST
{
    enum class LogLevel;
}

class Baseroot : public QObject
{
    Q_OBJECT

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

    signals:
        /**
         * @brief Signal emitted for all log messages
         * @param level Log level (Debug, Info, Warning, Error, Critical)
         * @param message Translation key for the message
         * @param args Dynamic arguments for parametric translation
         * @param context Context/source (module name, component, etc.)
         */
        void logSignal(OST::LogLevel level, const QString &message,
                       const QVariantList &args, const QString &context);

    protected:
        /**
         * @brief Helper methods to facilitate usage
         */
        void logDebug(const QString &message, const QVariantList &args = {});
        void logInfo(const QString &message, const QVariantList &args = {});
        void logWarning(const QString &message, const QVariantList &args = {});
        void logError(const QString &message, const QVariantList &args = {});
        void logCritical(const QString &message, const QVariantList &args = {});

        /**
         * @brief Returns the module/component name (to override in derived classes)
         */
        virtual QString getModuleName() const { return "Unknown"; }

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

