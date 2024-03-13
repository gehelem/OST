#ifndef ELEMENTINT_h
#define ELEMENTINT_h

#include <elementsingle.h>
#include <lovint.h>

namespace  OST
{

class ElementInt: public ElementSingleNumeric<int>
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

        ElementInt(const QString &label, const QString &order, const QString &hint);
        ~ElementInt();
        QString getType() override
        {
            return "int";
        }
        QMap<int, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(int val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(int  val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(int  val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }
    private:
        LovInt mLov;
};

class GridInt: public GridTemplate<int>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridInt(ElementBase* value): GridTemplate<int>(value), mElement(static_cast<ElementInt*>(value)) {}
        ~GridInt() {}

        QString getType() override
        {
            return "int";
        }
        ElementInt* mElement;
};

class ValueInt: public ValueTemplate<int>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueInt(ElementBase* element): ValueTemplate<int>(element) {}
        ~ValueInt() {}
};

}

#endif
