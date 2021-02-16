#ifndef MNAVIGATOR_H_
#define MNAVIGATOR_H_
#include "module.h"

class MNavigator : public Module
{
    Q_OBJECT
    public:
        MNavigator(Properties *properties);
        ~MNavigator();
        void initProperties(void);

    public slots:
        void slotvalueChangedFromCtl(Prop prop);

   private:
        bool sendRaDec(double ra, double dec);
        void startGoto(void);
        void abort(void);

        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;

};
#endif
