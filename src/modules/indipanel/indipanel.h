#ifndef INDI_PANEL_MODULE_h_
#define INDI_PANEL_MODULE_h_

#include "module.h"
#include "properties.h"

class IndiPanel : public Module
{
    Q_OBJECT

    public:
        IndiPanel();
        ~IndiPanel();

    signals:
        void valueChanged(const double &newValue);

    public slots:
        void OnIndiNewDevice          (INDI::BaseDevice *dp) override;
        void OnIndiRemoveDevice       (INDI::BaseDevice *dp) override;
        void OnIndiNewProperty        (INDI::Property *property) override;
        void OnIndiRemoveProperty     (INDI::Property *property) override;
        void OnIndiNewNumber(INumberVectorProperty *nvp) override;
        void OnIndiNewText(ITextVectorProperty *tvp) override;
        void OnIndiNewLight(ILightVectorProperty *lvp) override;
        void OnIndiNewBLOB(IBLOB *bp) override;
        void OnIndiNewSwitch(ISwitchVectorProperty *svp) override;
    private:

};
#endif
