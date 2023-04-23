#ifndef BASICPROPERTY_h_
#define BASICPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include "rootproperty.h"
namespace  OST
{


class PropertyVisitor;

class BasicProperty : public RootProperty
{

        Q_OBJECT

    public:
        BasicProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~BasicProperty();
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }

    signals:
        void valueChanged(void);
    private:

};
}
#endif

