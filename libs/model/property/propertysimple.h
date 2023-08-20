#ifndef PROPERTYSIMPLE_h_
#define PROPERTYSIMPLE_h_

#include <propertybase.h>
namespace  OST
{


class PropertySimple: public PropertyBase
{

        Q_OBJECT

    public:
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }


        PropertySimple(const QString &label, const Permission &permission, const QString &level1, const QString &level2,
                       const QString &order, const bool &hasProfile, const bool &hasArray
                      );
        ~PropertySimple();
        inline const ValueBase* getValue()
        {
            return mValue;
        }
        void setValue(QVariantMap &data);

    private:
        ValueBase* mValue = nullptr;

};

}
#endif

