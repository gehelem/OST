#ifndef ELEMENTMESSAGE_h
#define ELEMENTMESSAGE_h

#include <elementsingle.h>

namespace  OST
{

class ElementMessage: public ElementSingleNotNumeric<QString>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ElementMessage(const QString &label, const QString &order, const QString &hint);
        ~ElementMessage();
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

class ValueMessage: public ValueTemplate<QString>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueMessage(ElementBase* element): ValueTemplate<QString>(element) {}
        ~ValueMessage() {}
        void updateValue() override
        {
            value = static_cast<ElementMessage*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementMessage*>(pElement)->setValue(value, emitEvent);
        }

};
}
#endif
