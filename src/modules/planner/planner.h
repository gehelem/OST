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
         * @brief Start current planning line
         */
        void startLine();


        // Example internal state variables
        bool mIsRunning;

        // Waiting sequence
        bool mWaitingSequence ;

        // Waiting navigator
        bool mWaitingNavigator;

        // Current line
        int mCurrentLine;

};

// Required export function for module loading
extern "C" Planner* initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif // Planner_H
