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

    private:
        QString mState = "idle";
};

extern "C" MODULE_INIT Monitor *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
