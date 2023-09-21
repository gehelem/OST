#ifndef LOVFLOAT_h
#define LOVFLOAT_h

#include <lovsingle.h>

namespace  OST
{

class LovFloat: public LovSingle<double>
{

        Q_OBJECT

    public:
        void accept(LovVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        LovFloat(const QString &label): LovSingle<double>(label) {}
        ~LovFloat() {}
        QString getType() override
        {
            return "float";
        }

    private:

};

}
#endif
