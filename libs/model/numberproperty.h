#ifndef NUMBERPROPERTY_h_
#define NUMBERPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <rootproperty.h>


class NumberProperty : public RootProperty
{
        Q_OBJECT

    public:
        NumberProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~NumberProperty();
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        double value()
        {
            return mValue;
        }
        void setValue(double value);

    private:
        double mValue;

};
Q_DECLARE_METATYPE(NumberProperty*);
#endif

