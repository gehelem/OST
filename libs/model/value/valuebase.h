#ifndef VALUEBASE_h_
#define VALUEBASE_h_

#include <valuevisitor.h>
#include <common.h>

namespace  OST
{

class ValueVisitor;

class ValueBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(ValueVisitor* pVisitor) = 0;
        virtual void accept(ValueVisitor* pVisitor, QVariantMap &data) = 0;

        ValueBase(const QString &label, const QString &order, const QString &hint);
        ~ValueBase();

        QString label()
        {
            return mLabel;
        }
        QString order()
        {
            return mOrder;
        }
        QString hint()
        {
            return mHint;
        }
        State state()
        {
            return mState;
        }
        void setState(const State &state);
        bool autoUpdate()
        {
            return mAutoUpdate;
        }
        void setAutoUpdate(const bool &v)
        {
            mAutoUpdate = v;
        }
    private:
        QString mLabel = "change me";
        QString mOrder = "change me";
        QString mHint = "";
        State mState = State::Idle;
        bool mAutoUpdate = false;
    signals:
        void valueChanged(OST::ValueBase*);


};

}

#endif

