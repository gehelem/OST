#ifndef ROOTMULTI_h_
#define ROOTMULTI_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <rootproperty.h>


class RootMulti: public RootProperty
{
        Q_OBJECT

    public:
        RootMulti(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~RootMulti();
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

