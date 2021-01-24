#ifndef MFOCUSER_h_
#define MFOCUSER_h_
#include "module.h"

class MFocuser : public Module
{
    Q_OBJECT
    public:
        MFocuser(MyClient *cli,OSTProperties *properties);
        ~MFocuser();

        void initProperties(void);
        void startFraming(void);
        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;
        void slotvalueChangedFromCtl(elem el);
   public slots:
        void test(void);

};
#endif
