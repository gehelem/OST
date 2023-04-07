#ifndef ROOTPROPERTY_h_
#define ROOTPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include "propertyvisitor.h"
#include "rootvalue.h"


class PropertyVisitor;

class RootProperty : public RootValue
{

        Q_OBJECT

    public:
        virtual void accept(PropertyVisitor* pVisitor) = 0;
        virtual void accept(PropertyVisitor* pVisitor, QVariantMap &data) = 0;

        enum Permission { ReadOnly, WriteOnly, ReadWrite};
        Q_ENUM(Permission)
        enum State { Idle, Ok, Busy, Error};
        Q_ENUM(State)

        RootProperty(const QString &label, const QString &level1, const QString &level2,
                     const Permission &permission );
        ~RootProperty();

        QString level1()
        {
            return mLevel1;
        }
        QString level2()
        {
            return mLevel2;
        }
        Permission permission()
        {
            return mPermission;
        }

    signals:

    private:
        QString mLevel1;
        QString mLevel2;
        Permission mPermission;

};
Q_DECLARE_METATYPE(RootProperty*);
#endif

