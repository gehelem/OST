#ifndef MNAVIGATOR_H_
#define MNAVIGATOR_H_
#include "module.h"

class MNavigator : public Module
{
    Q_OBJECT
    public:
        MNavigator(MyClient *cli,OSTProperties *properties);
        ~MNavigator();

        void initProperties(void);
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
