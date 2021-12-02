#ifndef INDIPANEL_MODULE_h_
#define INDIPANEL_MODULE_h_
#include <basemodule.h>

#if defined(INDIPANEL_MODULE_h_)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT IndiPanel : public Basemodule
{
    Q_OBJECT

    public:
        IndiPanel(QString name,QString label);
        ~IndiPanel();
    signals:
        void valueChanged(const double &newValue);
    private:
        void newDevice      (INDI::BaseDevice *dp) override;
        void removeDevice   (INDI::BaseDevice *dp) override;
        void newProperty    (INDI::Property *property) override;
        void removeProperty (INDI::Property *property) override;
        void newNumber      (INumberVectorProperty *nvp) override;
        void newText        (ITextVectorProperty *tvp) override;
        void newLight       (ILightVectorProperty *lvp) override;
        void newBLOB        (IBLOB *bp) override;
        void newSwitch      (ISwitchVectorProperty *svp) override;
        void newMessage     (INDI::BaseDevice *dp, int messageID) override;

};

extern "C" MODULE_INIT IndiPanel *initialize(QString name,QString label);

#endif
