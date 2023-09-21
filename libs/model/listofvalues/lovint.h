#ifndef LOVINT_h
#define LOVINT_h

#include <lovsingle.h>

namespace  OST
{

class LovInt: public LovSingle<int>
{

        Q_OBJECT

    public:
        void accept(LovVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        LovInt(const QString &label): LovSingle<int>(label) {}
        ~LovInt() {}
        QString getType() override
        {
            return "int";
        }

    private:

};

}
#endif
