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
        QString label()
        {
            return mLabel;
        }
        QString order()
        {
            return mOrder;
        }
        QString hint()
        {
            return mHint;
        }
        bool autoUpdate()
        {
            return mAutoUpdate;
        }
        void setAutoUpdate(const bool &v)
        {
            mAutoUpdate = v;
        }
        bool getDirectEdit()
        {
            return mDirectEdit;
        }
        void setDirectEdit(const bool &v)
        {
            mDirectEdit = v;
        }
        bool getBadge()
        {
            return mBadge;
        }
        void setBadge(const bool &b)
        {
            mBadge = b;
        }
        QString getPreIcon()
        {
            return mPreIcon;
        }
        void setPreIcon(QString s)
        {
            mPreIcon = s;
        }
        QString getPostIcon()
        {
            return mPostIcon;
        }
        void setPostIcon(QString s)
        {
            mPostIcon = s;
        }
        QString getGlobalLov()
        {
            return mGlobalLov;
        }
        void setGlobalLov(QString lovName)
        {
            mGlobalLov = lovName;
        }
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
        void OnLovChanged()
        {
            emit lovChanged(this);
        }
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

