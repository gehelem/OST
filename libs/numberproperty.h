#ifndef NUMBERPROPERTY_h_
#define NUMBERPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <basicproperty.h>


class NumberProperty : public BasicProperty
{
        Q_OBJECT
        Q_PROPERTY(double value MEMBER mValue NOTIFY valueChanged)

    public:
        NumberProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~NumberProperty();
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }

    signals:
        void valueChanged(double);

    private:
        double mValue;

};
Q_DECLARE_METATYPE(NumberProperty*);
#endif

