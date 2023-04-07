#ifndef MULTIPROPERTY_h_
#define MULTIPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <rootproperty.h>
#include <rootmulti.h>


class MultiProperty : public RootMulti
{
        Q_OBJECT

    public:
        MultiProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~MultiProperty();
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }

        void addElt(QString name, RootValue* elt)
        {
            mElements[name] = elt;
        }
        QMap<QString, RootValue*> getElts(void)
        {
            return mElements;
        }

    signals:

    private:
        QMap<QString, RootValue*> mElements;


};

#endif

