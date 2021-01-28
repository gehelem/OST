#ifndef MFOCUSER_h_
#define MFOCUSER_h_
#include "module.h"

class MFocuser : public Module
{
    Q_OBJECT
    public:
        MFocuser(MyClient *cli,Properties *properties);
        ~MFocuser();

        void initProperties(void);
        void startFraming(void);
        void startCoarse(void);
        void startFine(void);
        void abort(void);
        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;
        void slotvalueChangedFromCtl(Prop prop);
   public slots:
        void test(void);

};
#endif
