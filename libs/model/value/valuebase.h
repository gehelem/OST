#ifndef VALUEBASE_h_
#define VALUEBASE_h_

#include <valuevisitor.h>

namespace  OST
{

class ValueVisitor;

typedef enum
{
    Idle = 0, /*!< State is idle */
    Ok,       /*!< State is ok */
    Busy,     /*!< State is busy */
    Error     /*!< State is error */
} State;

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

    private:
        QString mLabel = "change me";
        QString mOrder = "change me";
        QString mHint = "";
        State mState = State::Idle;

};

}

#endif

