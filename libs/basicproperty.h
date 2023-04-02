#ifndef BASICPROPERTY_h_
#define BASICPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include "propertyvisitor.h"


class PropertyVisitor;

class BasicProperty : public QObject
{

        Q_OBJECT
        Q_PROPERTY(QString label MEMBER mLabel)

    public:
        virtual void accept(PropertyVisitor* pVisitor) = 0;
        enum Permission { ReadOnly, WriteOnly, ReadWrite};
        Q_ENUM(Permission)
        enum State { Idle, Ok, Busy, Error};
        Q_ENUM(State)

        BasicProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~BasicProperty();

        void setState(State state);
        State state() const;

    signals:
        //void propertyCreated(BasicProperty);
        void stateChanged(BasicProperty::State);

    private:
        QString mLabel;
        QString mLevel1;
        QString mLevel2;
        Permission mPermission;
        State mState;

};
Q_DECLARE_METATYPE(BasicProperty*);
#endif

