#ifndef MONITOR_MODULE_h_
#define MONITOR_MODULE_h_
#include <indimodule.h>


#if defined(MONITOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Monitor : public IndiModule
{
        Q_OBJECT

    public:
        Monitor(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Monitor();
        void onNewDevice      (INDI::BaseDevice dp) override     {};
        void onRemoveDevice   (INDI::BaseDevice dp) override     {};
        void onNewProperty    (INDI::Property property) override {};
        void onRemoveProperty (INDI::Property property) override {};
        void onUpdateProperty (INDI::Property property) override {};

    protected:
        void onExternalEvent(OST::ExtEvent event) override;
        void onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line) override;

    private slots:
        void onAutoStart();

    private:
        bool isWatchedModule(const QString &mod);
        void appendEvent(const QString &module, const QString &type, const QString &key,
                         double valNum1 = 0, double valNum2 = 0, double valNum3 = 0,
                         double valNum4 = 0, double valNum5 = 0, double valNum6 = 0,
                         double valNum7 = 0, double valNum8 = 0,
                         int valInt1 = 0, int valInt2 = 0, int valInt3 = 0,
                         const QString &valStr1 = {}, const QString &valStr2 = {}, const QString &valStr3 = {});
        void startSession();
        void stopSession();
        void refreshView();
        void scanArchive();
        void loadArchive(int line);

        QDateTime             mSessionStart;
        QVector<QVariantMap>  mEvents;
        bool                  mSessionActive = false;

        // guideRMS / guideSNR accumulators
        QVector<double>       mGuideRmsBuf;
        QString               mGuideRmsModule;
        QString               mGuideRmsType;
        QVector<double>       mGuideSNRBuf;
        QString               mGuideSNRModule;
        QString               mGuideSNRType;
};

extern "C" MODULE_INIT Monitor *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
