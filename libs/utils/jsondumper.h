#ifndef OST_JSONDUMPER_H
#define OST_JSONDUMPER_H

#include <QString>
#include <propertyvisitor.h>
#include <numberproperty.h>
#include <basicproperty.h>
#include <multiproperty.h>
#include <propertyvisitor.h>

class JSonDumper : public PropertyVisitor
{


    public:
        void visit(RootProperty *pProperty) override
        {
            Q_UNUSED(pProperty)
        }
        void visit(BasicProperty *pProperty) override;
        void visit(NumberProperty *pProperty) override;
        void visit(TextProperty *pProperty) override;
        void visit(MultiProperty *pProperty) override;
        void visit(RootProperty* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        void visit(BasicProperty* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        void visit(NumberProperty* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        void visit(TextProperty* pProperty, QVariantMap &data ) override
        {
            Q_UNUSED(pProperty);
            Q_UNUSED(data)
        }
        [[nodiscard]] inline const QJsonObject &getResult() const
        {
            return _result;
        }
        QString &getStringResult()
        {
            QJsonDocument doc(_result);
            mJsonString = QString(doc.toJson(QJsonDocument::Compact));
            return mJsonString;
        }

    private:
        QJsonObject _result;
        QString mJsonString;
        QJsonObject dumpPropertyCommons(RootProperty* pProperty);
};


#endif //OST_JSONDUMPER_H
