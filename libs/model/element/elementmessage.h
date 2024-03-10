#ifndef VALUEMESSAGE_h
#define VALUEMESSAGE_h

#include <valuesingle.h>

namespace  OST
{

class ValueMessage: public ValueSingleNotNumeric<QString>
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
        void accept(ValueVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ValueMessage(const QString &label, const QString &order, const QString &hint);
        ~ValueMessage();
        QString getType() override
        {
            return "message";
        }
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

class GridMessage: public GridTemplate<QString>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridMessage(ValueBase* value): GridTemplate<QString>(value), mElement(static_cast<ValueMessage*>(value)) {}
        ~GridMessage() {}

        QString getType() override
        {
            return "message";
        }
        ValueMessage* mElement;
};

}
#endif
