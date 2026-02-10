#include "translatemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtXml/QDomDocument>
#include <QMutexLocker>
#include <QTextStream>

namespace OST
{

static TranslateManager *s_instance = nullptr;

TranslateManager::TranslateManager()
    : QObject(nullptr)
    , mCurrentLanguage("fr")
    , mQtTranslator(nullptr)
{
    // Initialize Qt translator
    mQtTranslator = new QTranslator(this);

    // Load existing pending entries for default language to avoid duplicates
    loadExistingPendingEntries(mCurrentLanguage);
}

TranslateManager::~TranslateManager()
{
    saveMissingTranslations();
}

TranslateManager* TranslateManager::instance()
{
    if (s_instance == nullptr)
    {
        s_instance = new TranslateManager();
    }
    return s_instance;
}

void TranslateManager::setLanguage(const QString &language)
{
    if (mCurrentLanguage == language)
        return;

    mCurrentLanguage = language;

    // Load pending entries for this language if not already loaded
    if (!mPendingFileEntriesByLang.contains(language))
    {
        loadExistingPendingEntries(language);
    }

    // Update Qt translator
    if (mQtTranslator && !mTranslationsPath.isEmpty())
    {
        // Remove old translation
        QCoreApplication::removeTranslator(mQtTranslator);

        // Load new translation
        QString qmFile = QString("ost_%1.qm").arg(language);
        QString qmPath = QDir(mTranslationsPath).filePath(qmFile);

        if (mQtTranslator->load(qmPath))
        {
            QCoreApplication::installTranslator(mQtTranslator);
            //qDebug() << "Loaded Qt translation:" << qmPath;
        }
        else
        {
            qWarning() << "Failed to load Qt translation:" << qmPath;
        }
    }

    emit languageChanged(language);
}

bool TranslateManager::loadQtTranslations(const QString &translationsPath)
{
    mTranslationsPath = translationsPath;

    // Load translation for current language
    QString qmFile = QString("ost_%1.qm").arg(mCurrentLanguage);
    QString qmPath = QDir(translationsPath).filePath(qmFile);

    if (QFile::exists(qmPath))
    {
        if (mQtTranslator->load(qmPath))
        {
            QCoreApplication::installTranslator(mQtTranslator);
            //qDebug() << "Loaded Qt translation:" << qmPath;
            return true;
        }
        else
        {
            qWarning() << "Failed to load Qt translation:" << qmPath;
            return false;
        }
    }
    else
    {
        qDebug() << "Qt translation file not found:" << qmPath << "(will use legacy only)";
        return false;
    }
}

QString TranslateManager::translate(const QString &sourceText, const char *context)
{
    if (sourceText.isEmpty())
        return sourceText;

    // Strategy 1: Try Qt Linguist translation first
    QString qtTranslation = QCoreApplication::translate(
                                context ? context : "OST",
                                sourceText.toUtf8().constData(),
                                nullptr,
                                -1
                            );

    // Check if Qt actually translated it (not just returned source)
    if (qtTranslation != sourceText)
    {
        // Qt found a translation
        return qtTranslation;
    }

    // Strategy 3: Mark as missing and return source
    if (!mMissingTranslations.contains(sourceText))
    {
        mMissingTranslations.insert(sourceText);
        //qDebug() << "Missing translation (" << mCurrentLanguage << "):" << sourceText;

        // Append to pending_<lang>.ts file in real-time
        appendToPendingFile(sourceText, context, mCurrentLanguage);
    }

    return sourceText;
}

QString TranslateManager::translate(const QString &sourceText, const char *context, int n)
{
    // For plural forms (not implemented yet, just forward to regular translate)
    Q_UNUSED(n);
    return translate(sourceText, context);
}

bool TranslateManager::hasQtTranslation(const QString &sourceText, const char *context) const
{
    QString qtTranslation = QCoreApplication::translate(
                                context ? context : "OST",
                                sourceText.toUtf8().constData(),
                                nullptr,
                                -1
                            );

    return (qtTranslation != sourceText);
}

void TranslateManager::saveMissingTranslations()
{
    if (mMissingTranslations.isEmpty())
        return;

    // Save missing translations to file for later migration
    QFile file("translations_missing_qt.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "# Missing Qt Linguist translations\n";
        out << "# Add these to your source code with tr() and run lupdate\n\n";

        for (const QString &text : mMissingTranslations)
        {
            out << text << "\n";
        }

        file.close();
        //qDebug() << "Saved" << mMissingTranslations.size() << "missing translations to translations_missing_qt.txt";
    }
}

QString TranslateManager::applyArgs(const QString &format, const QVariantList &args)
{
    QString result = format;

    for (const QVariant &arg : args)
    {
        switch (static_cast<QMetaType::Type>(arg.type()))
        {
            case QMetaType::Int:
            case QMetaType::LongLong:
                result = result.arg(arg.toLongLong());
                break;
            case QMetaType::UInt:
            case QMetaType::ULongLong:
                result = result.arg(arg.toULongLong());
                break;
            case QMetaType::Double:
            case QMetaType::Float:
                result = result.arg(arg.toDouble(), 0, 'f', 2);
                break;
            case QMetaType::Bool:
                result = result.arg(arg.toBool() ? "true" : "false");
                break;
            default:
                result = result.arg(arg.toString());
                break;
        }
    }

    return result;
}

QString TranslateManager::translateWithArgs(const QString &sourceText,
        const QVariantList &args,
        const QString &language)
{
    // Temporarily switch language if specified
    QString originalLanguage;
    if (!language.isEmpty() && language != mCurrentLanguage)
    {
        originalLanguage = mCurrentLanguage;
        setLanguage(language);
    }

    // Translate the format string
    QString translated = translate(sourceText, "OST");

    // Apply arguments
    QString result = applyArgs(translated, args);

    // Restore original language if changed
    if (!originalLanguage.isEmpty())
    {
        setLanguage(originalLanguage);
    }

    return result;
}

QString TranslateManager::getPendingFilePath(const QString &language) const
{
    return QString("translations_pending_%1.ts").arg(language);
}

void TranslateManager::loadExistingPendingEntries(const QString &language)
{
    QString pendingFilePath = getPendingFilePath(language);
    QFile file(pendingFilePath);

    if (!file.exists())
    {
        qDebug() << "Pending translations file not found, will create:" << pendingFilePath;
        mPendingFileEntriesByLang[language] = QSet<QString>();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open pending translations file:" << pendingFilePath;
        mPendingFileEntriesByLang[language] = QSet<QString>();
        return;
    }

    QXmlStreamReader xml(&file);
    QSet<QString> entries;
    int count = 0;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "message")
        {
            // Read until we find <source>
            while (!xml.atEnd())
            {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == "source")
                {
                    QString sourceText = xml.readElementText();
                    entries.insert(sourceText);
                    count++;
                    break;
                }
                if (xml.isEndElement() && xml.name() == "message")
                    break;
            }
        }
    }

    file.close();

    if (xml.hasError())
    {
        qWarning() << "XML parsing error in pending file:" << xml.errorString();
    }
    else
    {
        //qDebug() << "Loaded" << count << "existing pending translations from" << pendingFilePath;
    }

    mPendingFileEntriesByLang[language] = entries;
}

void TranslateManager::appendToPendingFile(const QString &sourceText, const char *context, const QString &language)
{
    QMutexLocker locker(&mPendingMutex);

    // Check if already in file for this language
    if (mPendingFileEntriesByLang[language].contains(sourceText))
    {
        return;  // Already recorded
    }

    // Mark as added to file
    mPendingFileEntriesByLang[language].insert(sourceText);

    QString contextStr = context ? QString(context) : "OST";
    QString pendingFilePath = getPendingFilePath(language);

    // Read existing file or create structure
    QFile file(pendingFilePath);
    QDomDocument doc;

    bool fileExists = file.exists();
    if (fileExists && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg;
        int errorLine, errorCol;
        if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol))
        {
            qWarning() << "Failed to parse" << pendingFilePath << ":" << errorMsg << "at line" << errorLine;
            file.close();
            return;
        }
        file.close();
    }
    else
    {
        // Create new XML structure
        QDomProcessingInstruction xmlDecl = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlDecl);

        QDomElement tsRoot = doc.createElement("TS");
        tsRoot.setAttribute("version", "2.1");
        tsRoot.setAttribute("language", language);
        doc.appendChild(tsRoot);

        QDomElement contextElem = doc.createElement("context");
        tsRoot.appendChild(contextElem);

        QDomElement nameElem = doc.createElement("name");
        nameElem.appendChild(doc.createTextNode(contextStr));
        contextElem.appendChild(nameElem);
    }

    // Find or create context
    QDomElement tsRoot = doc.documentElement();
    QDomElement contextElem;

    QDomNodeList contexts = tsRoot.elementsByTagName("context");
    bool contextFound = false;

    for (int i = 0; i < contexts.count(); ++i)
    {
        QDomElement ctx = contexts.at(i).toElement();
        QDomElement nameElem = ctx.firstChildElement("name");
        if (!nameElem.isNull() && nameElem.text() == contextStr)
        {
            contextElem = ctx;
            contextFound = true;
            break;
        }
    }

    if (!contextFound)
    {
        contextElem = doc.createElement("context");
        tsRoot.appendChild(contextElem);

        QDomElement nameElem = doc.createElement("name");
        nameElem.appendChild(doc.createTextNode(contextStr));
        contextElem.appendChild(nameElem);
    }

    // Create new message element
    QDomElement messageElem = doc.createElement("message");
    contextElem.appendChild(messageElem);

    QDomElement sourceElem = doc.createElement("source");
    sourceElem.appendChild(doc.createTextNode(sourceText));
    messageElem.appendChild(sourceElem);

    QDomElement translationElem = doc.createElement("translation");
    translationElem.setAttribute("type", "unfinished");
    messageElem.appendChild(translationElem);

    // Write back to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open" << pendingFilePath << "for writing";
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << doc.toString(4);
    file.close();

    qDebug() << "Added to" << pendingFilePath << ":" << sourceText;
}

} // namespace OST
