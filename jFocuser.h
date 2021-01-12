#ifndef JFOCUSER_h_
#define JFOCUSER_h_
#include "client.h"
#include "job.h"

class JFocuser : public Job
{
    Q_OBJECT
    public:
        JFocuser(MyClient *cli);
        ~JFocuser();

        void setProperties(void);
        void startFraming(void);
        virtual void executeTaskSpec(Ttask task) override;
        virtual void executeTaskSpec(Ttask task,IBLOB *bp) override;
        virtual void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) override;
        virtual void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) override;
        virtual void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) override;
        virtual void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) override;
};
#endif
