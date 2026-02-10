#ifndef TRANSLATEMANAGER_H
#define TRANSLATEMANAGER_H

#include <QtCore>
#include <QTranslator>
#include <QMutex>

namespace OST
{

/**
 * @brief Hybrid translation manager supporting both custom JSON and Qt Linguist
 *
 * Migration strategy:
 * 1. Try Qt Linguist translation first (QTranslator with tr())
 * 2. Fallback to legacy JSON translation if not found
 * 3. Mark missing translations for later migration
 *
 * Usage in C++ code:
 *   QString msg = TR("Camera");  // Simple translation
 *   QString msg = TR("iteration %1/%2").arg(current).arg(total);  // With parameters
 *
 * Usage before WebSocket send:
 *   QString translated = TranslateManager::translateForClient(originalText);
 */
class TranslateManager : public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Get singleton instance
         */
        static TranslateManager* instance();

        /**
         * @brief Set current language (e.g., "fr", "en")
         * @param language ISO 639-1 language code
         */
        void setLanguage(const QString &language);

        /**
         * @brief Get current language
         */
        QString currentLanguage() const
        {
            return mCurrentLanguage;
        }

        /**
         * @brief Translate text using Qt Linguist or fallback to legacy JSON
         *
         * This is the main translation method. It tries Qt Linguist first,
         * then falls back to the legacy Translate singleton.
         *
         * @param sourceText Text to translate (English by default)
         * @param context Translation context (usually class name)
         * @return Translated text in current language
         */
        QString translate(const QString &sourceText, const char *context = nullptr);

        /**
         * @brief Translate with placeholder arguments
         *
         * Example:
         *   translate("iteration %1/%2", "Focus").arg(3).arg(10)
         *   Result (fr): "itération 3/10"
         *
         * @param sourceText Text with %1, %2, etc. placeholders
         * @param context Translation context
         * @return Translated text (call .arg() on the result)
         */
        QString translate(const QString &sourceText, const char *context, int n);

        /**
         * @brief Check if Qt Linguist translation exists
         * @param sourceText Source text to check
         * @param context Translation context
         * @return true if Qt translation exists, false if would use legacy fallback
         */
        bool hasQtTranslation(const QString &sourceText, const char *context = nullptr) const;

        /**
         * @brief Load Qt translation files (.qm) from directory
         * @param translationsPath Path to directory containing .qm files
         * @return true if loaded successfully
         */
        bool loadQtTranslations(const QString &translationsPath);

        /**
         * @brief Translate and apply arguments in one call
         *
         * This method translates the source text to the target language,
         * then applies the arguments to create the final message.
         *
         * Example:
         *   translateWithArgs("iteration %1/%2", {3, 10}, "fr")
         *   Result: "itération 3/10"
         *
         * @param sourceText Text with %1, %2 placeholders
         * @param args List of arguments to substitute
         * @param language Target language (if empty, uses current language)
         * @return Fully translated and formatted text
         */
        QString translateWithArgs(const QString &sourceText,
                                  const QVariantList &args,
                                  const QString &language = QString());

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
        static QString applyArgs(const QString &format, const QVariantList &args);

    signals:
        /**
         * @brief Emitted when language changes
         * @param newLanguage New language code
         */
        void languageChanged(const QString &newLanguage);

    private:
        TranslateManager();
        ~TranslateManager();
        TranslateManager(const TranslateManager &) = delete;
        TranslateManager &operator=(const TranslateManager &) = delete;

        QString mCurrentLanguage;
        QTranslator *mQtTranslator;                // Qt Linguist translator
        QSet<QString> mMissingTranslations;         // Track missing translations (runtime)
        QMap<QString, QSet<QString>> mPendingFileEntriesByLang;  // Track pending entries per language
        QString mTranslationsPath;
        QMutex mPendingMutex;

        void saveMissingTranslations();  // Legacy (kept for compatibility)
        void loadExistingPendingEntries(const QString &language);  // Load existing pending_<lang>.ts
        void appendToPendingFile(const QString &sourceText, const char *context, const QString &language);  // Real-time append
        QString getPendingFilePath(const QString &language) const;  // Get pending file path for language
};

/**
 * @brief Convenience macro for translation with context
 *
 * Usage:
 *   QString msg = TR("Camera");
 *   QString msg = TR("iteration %1/%2").arg(current).arg(total);
 */
#define TR(text) TranslateManager::instance()->translate(text, "OST")

/**
 * @brief Translation macro with explicit context
 *
 * Usage in classes:
 *   QString msg = TR_CTX("Focus", "Camera not connected");
 */
#define TR_CTX(context, text) TranslateManager::instance()->translate(text, context)

} // namespace OST

#endif // TRANSLATEMANAGER_H
