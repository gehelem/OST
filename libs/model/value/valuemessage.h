#ifndef VALUEMESSAGE_h
#define VALUEMESSAGE_h

#include <valuebase.h>

namespace  OST
{
typedef enum
{
    Info = 0,
    Warn,
    Err
} MsgLevel;

class ValueMessage: public ValueBase
{

        Q_OBJECT

    public:
        void accept(ValueVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ValueVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }

        ValueMessage(const QString &label, const QString &order, const QString &hint);
        ~ValueMessage();
        QString message()
        {
            return mMessage;
        }
        void setMessage(const QString &message)
        {
            mMessage = message;
            mTimestamp = QDateTime::currentDateTime();
        }

        QDateTime timeStamp()
        {
            return mTimestamp;
        }

        MsgLevel level()
        {
            return mLevel;
        }
        void setLevel(const MsgLevel &level)
        {
            mLevel = level;
        }
    private:
        QString mMessage;
        QDateTime mTimestamp = QDateTime::currentDateTime();
        MsgLevel mLevel;

};

}
#endif
