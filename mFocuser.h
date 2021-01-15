#ifndef MFOCUSER_h_
#define MFOCUSER_h_
#include "module.h"

class MFocuser : public Module
{
    Q_OBJECT
    public:
        MFocuser(MyClient *cli);
        ~MFocuser();

        void initProperties(void);
        void startFraming(void);
        virtual void executeTaskSpec(Ttask task) override;
        virtual void executeTaskSpec(Ttask task,IBLOB *bp) override;
        virtual void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) override;
        virtual void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) override;
        virtual void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) override;
        virtual void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) override;
};
#endif
