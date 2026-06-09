#ifndef PARKMANAGER_MODULE_h_
#define PARKMANAGER_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#include <libastro.h>
#include <libnova/julian_day.h>

#define PI 3.14159265

#if defined(NAVIGATOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Parkmanager : public IndiModule
{
        Q_OBJECT

    public:
        Parkmanager(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Parkmanager();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property);

    signals:

        void cameraAlert();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:

        void initIndi(void);

        void SMAlert();

        QString mState = "idle";

};

extern "C" MODULE_INIT Parkmanager *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
