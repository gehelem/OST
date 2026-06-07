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

TranslateManager::TranslateManager()
{
    // Resources are initialized in main() before this constructor is called
}

TranslateManager::~TranslateManager()
{
    // Clean up all QTranslators
    for (QTranslator *translator : mTranslators.values())
    {
        QCoreApplication::removeTranslator(translator);
        delete translator;
    }
    mTranslators.clear();

    // Note: Qt resources are NOT cleaned up here because they might be used
    // by other parts of the application. They will be automatically freed
    // when the application exits.
}

bool TranslateManager::loadLanguages(const QStringList &languages)
{
    QMutexLocker locker(&mMutex);

    int successCount = 0;

    for (const QString &language : languages)
    {
        // Create QTranslator for this language
        QTranslator *translator = new QTranslator();

        // Load .qm file from embedded resources: :/translations/ost_<lang>.qm
        QString qmResourcePath = QString(":/translations/ost_%1.qm").arg(language);

        if (translator->load(qmResourcePath))
        {
            // Install translator globally (Qt uses all installed translators)
            QCoreApplication::installTranslator(translator);
            mTranslators[language] = translator;
            successCount++;
            qDebug() << "Loaded translation for language:" << language;
        }
        else
        {
            qWarning() << "Failed to load translation:" << qmResourcePath;
            delete translator;
        }

        // Load existing pending entries for this language (avoid duplicates)
        loadExistingPendingEntries(language);
    }

    qDebug() << "Loaded" << successCount << "out of" << languages.size() << "translation(s)";
    return successCount > 0;
}

QString TranslateManager::translate(const QString &sourceText, const QString &language)
{
    if (sourceText.isEmpty())
        return sourceText;

    QMutexLocker locker(&mMutex);

    // Check if translator exists for this language
    if (!mTranslators.contains(language))
    {
        qWarning() << "No translator loaded for language:" << language;
        // Mark as missing and return source
        if (!mMissingTranslationsByLang[language].contains(sourceText))
        {
            mMissingTranslationsByLang[language].insert(sourceText);
            // appendToPendingFile will be called with mutex already held
            appendToPendingFile(sourceText, language);
        }
        return sourceText;
    }

    // Try to translate using the specific language's QTranslator
    // Qt uses all installed translators, but we need to check this specific one
    QString translation = mTranslators[language]->translate("OST", sourceText.toUtf8().constData());

    // Check if translation was found (QTranslator returns empty string if not found)
    if (!translation.isEmpty())
    {
        return translation;
    }

    // Translation not found - mark as missing
    if (!mMissingTranslationsByLang[language].contains(sourceText))
    {
        mMissingTranslationsByLang[language].insert(sourceText);
        //qDebug() << "Missing translation (" << language << "):" << sourceText;

        // appendToPendingFile will be called with mutex already held
        appendToPendingFile(sourceText, language);
    }

    return sourceText;
}

QString TranslateManager::translateWithArgs(const QString &sourceText,
        const QVariantList &args,
        const QString &language)
{
    // Translate the format string
    QString translated = translate(sourceText, language);

    // Apply arguments
    QString result = applyArgs(translated, args);

    return result;
}

QString TranslateManager::applyArgs(const QString &format, const QVariantList &args)
{
    QString result = format;

    for (const QVariant &arg : args)
    {
        switch (static_cast<QMetaType::Type>(arg.typeId()))
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
                result = result.arg(arg.toDouble(), 0, 'f', 6);
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

        if (xml.isStartElement() && xml.name() == u"message")
        {
            // Read until we find <source>
            while (!xml.atEnd())
            {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"source")
                {
                    QString sourceText = xml.readElementText();
                    entries.insert(sourceText);
                    count++;
                    break;
                }
                if (xml.isEndElement() && xml.name() == u"message")
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
        qDebug() << "Loaded" << count << "existing pending translations from" << pendingFilePath;
    }

    mPendingFileEntriesByLang[language] = entries;
}

void TranslateManager::appendToPendingFile(const QString &sourceText, const QString &language)
{
    // NOTE: This method is called with mMutex already locked by the caller

    // Check if already in file for this language
    if (mPendingFileEntriesByLang[language].contains(sourceText))
    {
        return;  // Already recorded
    }

    // Mark as added to file
    mPendingFileEntriesByLang[language].insert(sourceText);

    QString contextStr = "OST";  // Fixed context
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
    stream << doc.toString(4);
    file.close();

    //qDebug() << "Added to" << pendingFilePath << ":" << sourceText;
}

} // namespace OST
