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
    QString message;         /*!< translated to the server language, used for console/file output */
    QString sourceMessage;   /*!< translation key / source text with placeholders, for re-translation */
    QVariantList args;       /*!< arguments to substitute into sourceMessage */

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
         * @brief Log a message, keeping the untranslated source and its args
         *        so the history can be re-translated later for any client
         * @param level Log level
         * @param message Message to log (already translated to server language)
         * @param context Optional context
         * @param sourceMessage Translation key / source text with placeholders
         * @param args Arguments to substitute into sourceMessage
         */
        void log(LogLevel level, const QString &message, const QString &context,
                 const QString &sourceMessage, const QVariantList &args);

        /**
         * @brief Get history, translated to the server language (legacy/console use)
         */
        QJsonArray getJsonHistory(void);

        /**
         * @brief Get history, re-translated to the given language
         * @param language Language to translate each entry into
         */
        QJsonArray getJsonHistory(const QString &language);
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

