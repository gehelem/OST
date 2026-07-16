#ifndef PLANNER_H
#define PLANNER_H

#include <indimodule.h>

/**
 * @brief PLanner module -
 *
 * Scheduling list :
 * - A target
 * - A sequencer profile
 * - Constraints
 */
class Planner : public IndiModule
{
        Q_OBJECT

    public:
        Planner(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Planner();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;
    public slots:
        void onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line) override;

    private slots:
        /**
         * @brief Example slot for handling async operations
         */
        void onOperationComplete();

    private:
        /**
         * @brief Initialize module-specific properties and devices
         */
        void initProperties();

        /**
         * @brief Start the main operation (adapt to your needs)
         */
        void startPlanner();

        /**
         * @brief Stop/abort the current operation
         */
        void abortPlanner();

        /**
         * @brief What to do after sequence is completed
         */
        void sequenceComplete();

        /**
         * @brief What to do after navigator is completed
         */
        void navigatorComplete();

        /**
         * @brief Start current planning line: kicks off the visibility check
         */
        void startLine();

        /**
         * @brief Slew and start the sequence, once the line has passed the visibility check
         */
        void proceedToSlew();

        /**
         * @brief Mark the current line as skipped (or failed, past the retry limit) and move on
         */
        void skipLine(const QString &reason);

        /**
         * @brief Check whether a target stays above the configured minimum elevation
         * for the whole estimated sequence duration, and clear of the moon.
         * @param reason filled with a human-readable explanation when returning false
         */
        bool checkVisibility(double ra, double dec, double durationSeconds, QString &reason);

        /**
         * @brief Move on to the next line still worth attempting (not Finished/Failed),
         * wrapping around the grid so skipped lines get retried once every other line
         * has had its turn. Ends the planning when nothing is left to attempt.
         */
        void advanceToNextLine();


        // Example internal state variables
        bool mIsRunning = false;

        // Waiting sequence
        bool mWaitingSequence = false ;

        // Waiting navigator
        bool mWaitingNavigator = false ;

        // Waiting for the sequencer's profileduration query answer
        bool mWaitingDuration = false ;

        // Current line
        int mCurrentLine;

        // Number of times each line (by grid index) has been skipped this run
        QMap<int, int> mSkipCounts;

};

// Required export function for module loading
extern "C" Planner* initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif // Planner_H
