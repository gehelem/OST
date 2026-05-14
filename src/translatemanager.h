#ifndef TRANSLATEMANAGER_H
#define TRANSLATEMANAGER_H

#include <QtCore>
#include <QTranslator>
#include <QMutex>

namespace OST
{

/**
 * @brief Multi-language translation manager supporting concurrent clients
 *
 * This manager loads multiple languages in parallel at startup and provides
 * thread-safe translation services without global state. Each translation
 * request explicitly specifies the target language.
 *
 * Key features:
 * - No singleton pattern (can create multiple instances)
 * - No default language (each call specifies language explicitly)
 * - All languages loaded in parallel (QTranslator per language)
 * - Thread-safe for concurrent multi-client usage
 * - Automatic detection of missing translations per language
 *
 * Usage:
 *   TranslateManager manager;
 *   manager.loadLanguages("/path/to/translations", {"fr", "en", "de"});
 *   QString msg = manager.translateWithArgs("iteration %1/%2", {3, 10}, "fr");
 *   // Result: "itération 3/10"
 */
class TranslateManager
{
    public:
        /**
         * @brief Constructor (public, no singleton)
         */
        TranslateManager();

        /**
         * @brief Destructor - cleans up all loaded translators
         */
        ~TranslateManager();

        // Delete copy/move constructors
        TranslateManager(const TranslateManager &) = delete;
        TranslateManager &operator=(const TranslateManager &) = delete;

        /**
         * @brief Load multiple languages from embedded resources
         *
         * Loads all specified language files (.qm) from Qt resources (:/translations/).
         * The .qm files are embedded in the executable at compile time.
         * This should be called once at startup.
         *
         * @param languages List of ISO 639-1 language codes (e.g., {"fr", "en", "de"})
         * @return true if at least one language loaded successfully
         */
        bool loadLanguages(const QStringList &languages);

        /**
         * @brief Translate and apply arguments in one call (MAIN API)
         *
         * This is the primary translation method. It translates the source text
         * to the target language, then applies the arguments to create the final message.
         * Missing translations are automatically detected and logged.
         *
         * Example:
         *   translateWithArgs("iteration %1/%2", {3, 10}, "fr")
         *   Result: "itération 3/10"
         *
         * @param sourceText Text with %1, %2 placeholders (English source)
         * @param args List of arguments to substitute (can be empty)
         * @param language Target language code (REQUIRED, no default)
         * @return Fully translated and formatted text
         */
        QString translateWithArgs(const QString &sourceText,
                                  const QVariantList &args,
                                  const QString &language);

        /**
         * @brief Apply arguments to a format string
         *
         * Static utility method to substitute %1, %2, etc. with actual values.
         * Handles different types (int, float, string, bool) appropriately.
         *
         * @param format String with %1, %2 placeholders
         * @param args Arguments to substitute
         * @return Formatted string
         */
        static QString applyArgs(const QString &format, const QVariantList &args = {});

    private:
        // Map of QTranslator per language (all loaded in memory)
        QMap<QString, QTranslator*> mTranslators;

        // Track missing translations per language (runtime detection)
        QMap<QString, QSet<QString>> mMissingTranslationsByLang;

        // Track pending file entries per language (avoid duplicates)
        QMap<QString, QSet<QString>> mPendingFileEntriesByLang;

        QMutex mMutex;  // Protect concurrent access

        /**
         * @brief Internal translation method
         * @param sourceText Text to translate
         * @param language Target language
         * @return Translated text (or source if not found)
         */
        QString translate(const QString &sourceText, const QString &language);

        /**
         * @brief Load existing pending entries to avoid duplicates
         * @param language Target language
         */
        void loadExistingPendingEntries(const QString &language);

        /**
         * @brief Append missing translation to pending_<lang>.ts file
         * @param sourceText Missing source text
         * @param language Target language
         */
        void appendToPendingFile(const QString &sourceText, const QString &language);

        /**
         * @brief Get pending file path for language
         * @param language Target language
         * @return Path to translations_pending_<lang>.ts
         */
        QString getPendingFilePath(const QString &language) const;
};

} // namespace OST

#endif // TRANSLATEMANAGER_H
