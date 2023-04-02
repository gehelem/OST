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
        double value()
        {
            return mValue;
        }
        void setValue(double value);

    signals:
        void valueChanged(double);

    private:
        double mValue;

};
Q_DECLARE_METATYPE(NumberProperty*);
#endif

