#ifndef LOVLIGHT_h
#define LOVLIGHT_h

#include <lovsingle.h>

namespace  OST
{

class LovLight: public LovSingle<State>
{

        Q_OBJECT

    public:
        void accept(LovVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        LovLight(const QString &label): LovSingle<State>(label) {}
        ~LovLight() {}
        QString getType() override
        {
            return "light";
        }

    private:

};

}
#endif
