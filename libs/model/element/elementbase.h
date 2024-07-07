#ifndef ELEMENTBASE_h_
#define ELEMENTBASE_h_

#include <elementvisitor.h>
#include <common.h>

namespace  OST
{

class ElementVisitor;
class ValueVisitor;


class ElementBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(ElementVisitor* pVisitor) = 0;
        virtual void accept(ElementVisitor* pVisitor, QVariantMap &data) = 0;
        virtual void accept(ElementVisitor* pVisitor, QString &action, QVariantMap &data) = 0;

        ElementBase(const QString &label, const QString &order, const QString &hint);
        ~ElementBase();

        virtual QString getType() = 0;
        QString label();
        QString order();
        QString hint();
        bool autoUpdate();
        void setAutoUpdate(const bool &v);
        bool getDirectEdit();
        void setDirectEdit(const bool &v);
        bool getBadge();
        void setBadge(const bool &b);
        QString getPreIcon();
        void setPreIcon(QString s);
        QString getPostIcon();
        void setPostIcon(QString s);
        QString getGlobalLov();
        void setGlobalLov(QString lovName);

    private:
        QString mLabel = "change me";
        QString mOrder = "change me";
        QString mHint = "";
        bool mAutoUpdate = false;
        bool mDirectEdit = false;
        bool mBadge = false;
        QString mPreIcon = "";
        QString mPostIcon = "";
        QString mGlobalLov = "";

    public slots:
        void OnLovChanged();
    signals:
        void valueSet(OST::ElementBase*); /* used in derived template class, only when value is changed*/
        void eltChanged(OST::ElementBase*); /* used in derived template class, when anything other than value is changed */
        void listChanged(OST::ElementBase*);
        void sendMessage(MsgLevel, QString);
        void lovChanged(OST::ElementBase*);
        void gridEvent();

};

class ValueBase: public QObject
{
        Q_OBJECT
    public:
        virtual void accept(ValueVisitor* pVisitor) = 0;
        ValueBase(ElementBase* element);
        ~ValueBase();
        virtual void updateValue() = 0;
        virtual void updateElement(const bool &emitEvent) = 0;
    protected:
        ElementBase* pElement;
};

}

#endif

