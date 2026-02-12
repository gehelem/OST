#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QVariantList>
#include <common.h>

namespace  OST
{

// Forward declaration
class TranslateManager;
struct LogMess
{
    LogLevel level = LogLevel::Info;
    QDateTime datetime;
    QString context;
    QString message;

};
/**
 * @brief Logger
 *
 * Usage:
 *   Logger.info("Information message");
 *   LOG_INFO("Information message"); // macro
 */
class Logger : public QObject
{
        Q_OBJECT

    public:
        Logger();
        ~Logger();

        /**
         * @brief Log debug message
         * @param message Message to log
         * @param context Optional context (class, function)
         */
        void debug(const QString &message, const QString &context = QString());

        /**
         * @brief Log informations message
         * @param message Message to log
         * @param context Optional context (class, function)
         */
        void info(const QString &message, const QString &context = QString());

        /**
         * @brief Log worning message
         * @param message Message to log
         * @param context Optional context (class, function)
         */
        void warning(const QString &message, const QString &context = QString());

        /**
         * @brief Log error message
         * @param message Message to log
         * @param context Optional context (class, function)
         */
        void error(const QString &message, const QString &context = QString());

        /**
         * @brief Log critical message
         * @param message Message to log
         * @param context Optional context (class, function)
         */
        void critical(const QString &message, const QString &context = QString());

        /**
         * @brief Define minimum log level
         * @param level minimum log level
         */
        void setLogLevel(LogLevel level);

        /**
         * @brief Enable/disable file logging
         * @param enabled true to activate file writing
         * @param filePath log file path
         */
        void setFileLogging(bool enabled, const QString &filePath = "ostserver.log");

        /**
         * @brief Enable/disable timestamp prefix log
         * @param enabled true to activate
         */
        void setTimestampEnabled(bool enabled);

        /**
         * @brief Log a message
         * @param level Log level
         * @param message Message to log
         */
        void log(LogLevel level, const QString &message);

        /**
         * @brief Log a message
         * @param level Log level
         * @param message Message to log
         * @param context Optinal context
         */
        void log(LogLevel level, const QString &message, const QString &context);

        /**
         * @brief Get history
         */
        QJsonArray getJsonHistory(void);
        /**
         * @brief Glear history
         */
        void clearHistory(void)
        {
            mHistory.clear();
        };

        /**
         * @brief Configure TranslateManager to translate messages
         * @param translator Pointer to TranslateManager
         * @param language Language for server console
         */
        void setTranslateManager(OST::TranslateManager* translator, const QString& language);

    public slots:
        /**
         * @brief Slot that listens to the universal logSignal() signal
         * @param level Log level
         * @param message Translation key
         * @param args Arguments for parametric translation
         * @param context Message context/source
         */
        void onLog(OST::LogLevel level, const QString &message,
                   const QVariantList &args, const QString &context);

    private:

        /**
         * @brief Get log level prefix
         * @param level Log level
         * @return Formated prefix (ex: "[DEBUG]")
         */
        QString getLevelPrefix(LogLevel level) const;

        /**
         * @brief Write into file
         * @param formattedMessage Formated message
         */
        void writeToFile(const QString &formattedMessage);

        LogLevel mLogLevel;
        bool mTimestampEnabled;
        bool mFileLoggingEnabled;
        QString mLogFilePath;
        QFile* pLogFile;
        QTextStream* pLogStream;
        QList<LogMess> mHistory;
        OST::TranslateManager* mTranslater = nullptr;
        QString mServerLanguage;
};

}

#endif // LOGGER_H

