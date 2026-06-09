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
        void onNewDevice      (INDI::BaseDevice ) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice ) override     {} ;
        void onNewProperty    (INDI::Property ) override {} ;
        void onRemoveProperty (INDI::Property ) override {} ;
        void onUpdateProperty (INDI::Property property)override;
        void onAfterInit(void) override;

    signals:

        void cameraAlert();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:
        void onTimer(void);
        void refreshDriversData(void);
        void initIndi(void);
        void calculateSunset(void);


};

extern "C" MODULE_INIT Parkmanager *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
