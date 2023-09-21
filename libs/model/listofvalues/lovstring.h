#ifndef LOVSTRING_h
#define LOVSTRING_h

#include <lovsingle.h>

namespace  OST
{

class LovString: public LovSingle<QString>
{

        Q_OBJECT

    public:
        void accept(LovVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        LovString(const QString &label): LovSingle<QString>(label) {}
        ~LovString() {}
        QString getType() override
        {
            return "string";
        }

    private:

};

}
#endif
