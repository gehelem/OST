#ifndef MSEQUENCE_H_
#define MSEQUENCE_H_
#include "module.h"

class MSequence : public Module
{
    Q_OBJECT
    public:
        MSequence(MyClient *cli,Properties *properties);
        ~MSequence();

        void initProperties(void);
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
