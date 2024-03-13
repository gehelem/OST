#ifndef ELEMENTSTRING_h
#define ELEMENTSTRING_h
#include <elementsingle.h>
#include <lovstring.h>
namespace  OST
{

class ElementString: public ElementSingleNotNumeric<QString>
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

        ElementString(const QString &label, const QString &order, const QString &hint);
        ~ElementString();
        QString getType() override
        {
            return "string";
        }
        QMap<QString, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(QString val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(QString  val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(QString  val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }
    private:
        LovString mLov;

};

class GridString: public GridTemplate<QString>
{

        Q_OBJECT

    public:
        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridString(ElementBase* value): GridTemplate<QString>(value), mElement(static_cast<ElementString*>(value)) {}
        ~GridString() {}

        QString getType() override
        {
            return "string";
        }
        ElementString* mElement;
};

class ValueString: public ValueTemplate<QString>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueString(ElementBase* element): ValueTemplate<QString>(element) {}
        ~ValueString() {}
        void updateValue() override
        {
            value = static_cast<ElementString*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementString*>(pElement)->setValue(value, emitEvent);
        }

};

}
#endif
