#ifndef PROPERTYMULTI_h_
#define PROPERTYMULTI_h_

#include <propertybase.h>
namespace  OST
{

typedef enum
{
    OneOfMany = 0,
    AtMostOne,
    Any
} SwitchsRule;


class PropertyMulti: public PropertyBase
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


        PropertyMulti(const QString &label, const Permission &permission, const QString &level1, const QString &level2,
                      const QString &order, const bool &hasProfile, const bool &hasArray
                     );
        ~PropertyMulti();
        [[nodiscard]] SwitchsRule getRule() const
        {
            return mRule;
        }
        inline const QMap<QString, OST::ValueBase*> &getValues()
        {
            return mValues;
        }
        void addValue(QString key, ValueBase* pValue)
        {
            if (mValues.contains(key))
            {
                qDebug() << label() << " - addValue - element " << key << " already exists";

            }
            else
            {
                mValues[key] = pValue;
            }

        }
    private:
        SwitchsRule mRule;
        QMap<QString, OST::ValueBase*> mValues;

};

}
#endif

