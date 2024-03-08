#ifndef VALUEBASE_h_
#define VALUEBASE_h_

#include <valuevisitor.h>
#include <common.h>

namespace  OST
{

class ValueVisitor;
class GridVisitor;

class ValueBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(ValueVisitor* pVisitor) = 0;
        virtual void accept(ValueVisitor* pVisitor, QVariantMap &data) = 0;
        virtual void accept(ValueVisitor* pVisitor, QString &action, QVariantMap &data) = 0;

        ValueBase(const QString &label, const QString &order, const QString &hint);
        ~ValueBase();

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
    private:
        QString mLabel = "change me";
        QString mOrder = "change me";
        QString mHint = "";
        bool mAutoUpdate = false;
        bool mDirectEdit = false;
        bool mBadge = false;
    public slots:
        void OnLovChanged()
        {
            emit lovChanged(this);
        }
    signals:
        void valueSet(OST::ValueBase*); /* used in derived template class, only when value is changed*/
        void valueChanged(OST::ValueBase*); /* used in derived template class, when anything other than value is changed */
        void listChanged(OST::ValueBase*);
        void sendMessage(MsgLevel, QString);
        void lovChanged(OST::ValueBase*);
        void gridEvent();

};

class GridBase: public QObject
{
        Q_OBJECT
    public:
        virtual void accept(GridVisitor* pVisitor, QVariant &data) = 0;

        GridBase(ValueBase* value) : mValue(value)
        {
        }
        ~GridBase();
        virtual QString getType() = 0;
    protected:
        ValueBase* mValue;

};




}

#endif

