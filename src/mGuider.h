#ifndef MGUIDER_H_
#define MGUIDER_H_
#include "module.h"

class MGuider : public Module
{
    Q_OBJECT
    public:
        MGuider(Properties *properties);
        ~MGuider();
        void initProperties(void);

    public slots:
        void slotvalueChangedFromCtl(Prop prop);

    private:
        void startCalibration(void);
        void abort(void);

        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;


};
#endif
