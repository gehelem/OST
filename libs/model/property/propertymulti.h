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
inline SwitchsRule IntToRule(int val )
{
    if (val == 0) return OneOfMany;
    if (val == 1) return AtMostOne;
    if (val == 2) return Any;
    qDebug() << "Cant convert " << val << " to OST::SwitchsRule (0-2) - defaults to any";
    return Any;
}


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


        PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                      const QString &level2,
                      const QString &order, const bool &hasProfile, const bool &hasArray
                     );
        ~PropertyMulti();
        [[nodiscard]] SwitchsRule rule() const
        {
            return mRule;
        }
        void setRule(SwitchsRule rule)
        {
            mRule = rule;
        }

        QMap<QString, OST::ValueBase*> getValues()
        {
            return mValues;
        }
        OST::ValueBase* getValue(QString pElement)
        {
            if (!mValues.contains(pElement))
            {
                qDebug() << label() << " - getValue - element " << pElement << " does not exists.";
                return nullptr;
            }
            return mValues[pElement];
        }
        void addValue(QString key, ValueBase* pValue)
        {
            if (mValues.contains(key))
            {
                qDebug() << label() << " - addValue - element " << key << " already exists";
                return;
            }
            //qDebug() << label() << " - addValue - element " << key << " OK " << pValue;
            mValues[key] = pValue;
            connect(mValues[key], &ValueBase::valueChanged, this, &PropertyMulti::OnValueChanged);
        }
        void push();
        void newLine(const QVariantMap &pValues);
        void deleteLine(const int i);
        void updateLine(const int i, const QVariantMap &pValues);
        void clearGrid();
    public slots:
        void OnValueChanged(ValueBase*)
        {
            emit valueChanged(this);
        }

    private:
        SwitchsRule mRule = SwitchsRule::Any;
        QMap<QString, ValueBase*> mValues;

};

}
#endif

