#ifndef ELEMENTSTRING_h
#define ELEMENTSTRING_h
#include <elementtemplate.h>
#include <lovstring.h>
namespace  OST
{

class ElementString: public ElementTemplateNotNumeric<QString>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override;
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        ElementString(const QString &label, const QString &order, const QString &hint);
        ~ElementString();
        QString getType() override;
        QMap<QString, QString> getLov();
        bool lovAdd(QString val, QString label);
        bool lovUpdate(QString  val, QString label);
        bool lovDel(QString  val);
        bool lovClear();

    private:
        LovString mLov;

};

class ValueString: public ValueTemplate<QString>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override;
        ValueString(ElementBase* element): ValueTemplate<QString>(element) {}
        ~ValueString() {}
        void updateValue() override;
        void updateElement(const bool &emitEvent) override;

};

}
#endif
