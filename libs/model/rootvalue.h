#ifndef ROOTVALUE_h_
#define ROOTVALUE_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include "propertyvisitor.h"
namespace  OST
{


class PropertyVisitor;

class RootValue: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(PropertyVisitor* pVisitor) = 0;
        virtual void accept(PropertyVisitor* pVisitor, QVariantMap &data) = 0;

        enum State { Idle, Ok, Busy, Error};
        Q_ENUM(State)

        RootValue(const QString &label);
        ~RootValue();

        QString label()
        {
            return mLabel;
        }
        QString order()
        {
            return mOrder;
        }
        void setOrder(QString &pOrder)
        {
            mOrder = pOrder;
        }
        State state()
        {
            return mState;
        }
        void setState(State state);

    signals:
        void stateChanged(RootValue::State);
        void propertyCreated(void);
        void valueChanged(RootValue*);


    private:
        QString mLabel;
        QString mOrder;
        State mState;

};

}
#endif

