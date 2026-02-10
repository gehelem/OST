#include "logger.h"
#include <QCoreApplication>
#include <QDebug>
#include <qjsonarray.h>
#include <qjsonobject.h>

namespace  OST
{

Logger::Logger()
    : QObject(nullptr)
    , mLogLevel(LogLevel::Debug)
    , mTimestampEnabled(true)
    , mFileLoggingEnabled(false)
    , pLogFile(nullptr)
    , pLogStream(nullptr)
{
}

Logger::~Logger()
{
}

void Logger::debug(const QString &message, const QString &context)
{
    log(LogLevel::Debug, message, context);
}

void Logger::info(const QString &message, const QString &context)
{
    log(LogLevel::Info, message, context);
}

void Logger::warning(const QString &message, const QString &context)
{
    log(LogLevel::Warning, message, context);
}

void Logger::error(const QString &message, const QString &context)
{
    log(LogLevel::Error, message, context);
}
void Logger::critical(const QString &message, const QString &context)
{
    log(LogLevel::Critical, message, context);
}
void Logger::setLogLevel(LogLevel level)
{
    mLogLevel = level;
}

void Logger::setFileLogging(bool enabled, const QString &filePath)
{
    if (pLogStream)
    {
        delete pLogStream;
        pLogStream = nullptr;
    }
    if (pLogFile)
    {
        if (pLogFile->isOpen())
        {
            pLogFile->close();
        }
        delete pLogFile;
        pLogFile = nullptr;
    }

    mFileLoggingEnabled = enabled;
    mLogFilePath = filePath;

    if (enabled)
    {
        pLogFile = new QFile(filePath);
        if (pLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            pLogStream = new QTextStream(pLogFile);
        }
        else
        {
            qCritical() << QCoreApplication::translate("Logger", "Can't open logfile") << filePath;
            delete pLogFile;
            pLogFile = nullptr;
            mFileLoggingEnabled = false;
        }
    }
}

void Logger::setTimestampEnabled(bool enabled)
{
    mTimestampEnabled = enabled;
}
void Logger::log(LogLevel level, const QString &message)
{
    log(level, message, "");
}
void Logger::log(LogLevel level, const QString &message, const QString &context)
{
    if (level < mLogLevel)
    {
        return;
    }

    QString formattedMessage;
    QDateTime dt = QDateTime::currentDateTime();

    if (mTimestampEnabled)
    {
        formattedMessage += dt.toString("[yyyy-MM-dd HH:mm:ss.zzz] ");
    }

    formattedMessage += getLevelPrefix(level) + " ";

    if (!context.isEmpty())
    {
        formattedMessage += "[" + context + "] ";
    }

    formattedMessage += message;

    mHistory.append({level, dt, context, message});
    if (mHistory.size() > 100) mHistory.removeFirst();

    switch (level)
    {
        case LogLevel::Debug:
            qDebug().noquote() << formattedMessage;
            break;
        case LogLevel::Info:
            qInfo().noquote() << formattedMessage;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << formattedMessage;
            break;
        case LogLevel::Error:
            qCritical().noquote() << formattedMessage;
            break;
        case LogLevel::Critical:
            qCritical().noquote() << formattedMessage;
            break;
    }

    writeToFile(formattedMessage);

}

QString Logger::getLevelPrefix(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::Debug:
            return QCoreApplication::translate("Logger", "[DEBUG]");
        case LogLevel::Info:
            return QCoreApplication::translate("Logger", "[INFO]");
        case LogLevel::Warning:
            return QCoreApplication::translate("Logger", "[WARNING]");
        case LogLevel::Error:
            return QCoreApplication::translate("Logger", "[ERROR]");
        case LogLevel::Critical:
            return QCoreApplication::translate("Logger", "[CRITICAL]");
        default:
            return QCoreApplication::translate("Logger", "[UNKNOWN]");
    }
}

void Logger::writeToFile(const QString &formattedMessage)
{
    if (mFileLoggingEnabled)
    {
        pLogFile = new QFile(mLogFilePath);
        if (pLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            pLogStream = new QTextStream(pLogFile);
            (*pLogStream) << formattedMessage << Qt::endl;
            pLogFile->close();
        }
        else
        {
            qCritical() << QCoreApplication::translate("Logger", "Can't open logfile") << mLogFilePath;
            delete pLogFile;
            pLogFile = nullptr;
            mFileLoggingEnabled = false;
        }
    }

}
QJsonArray Logger::getJsonHistory(void)
{
    QJsonArray j;
    QList<LogMess>::iterator i;
    for (i = mHistory.begin(); i != mHistory.end(); ++i)
    {
        if (i->context != "WS")
        {
            QJsonObject o;
            o["d"] = i->datetime.toString("yyyy-MM-dd HH:mm:ss.zzz");
            o["c"] = i->context;
            o["l"] = static_cast<int>(i->level);
            o["t"] = i->message;
            j.append(o);
        };
    }
    return j;
};

}
