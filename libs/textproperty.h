#ifndef TEXTPROPERTY_h_
#define TEXTPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>
#include <rootproperty.h>


class TextProperty : public RootProperty
{
        Q_OBJECT

    public:
        TextProperty(const QString &label, const QString &level1, const QString &level2, const Permission &permission );
        ~TextProperty();
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        QString value()
        {
            return mValue;
        }
        void setValue(QString value);



    signals:
        void valueChanged(QString);

    private:
        QString mValue;

};
Q_DECLARE_METATYPE(TextProperty*);
#endif

