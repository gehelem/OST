#ifndef VALUEBASE_h_
#define VALUEBASE_h_

#include <valuevisitor.h>

namespace  OST
{

class ValueVisitor;

typedef enum
{
    Idle = 0, /*!< State is idle */
    Ok,       /*!< State is ok */
    Busy,     /*!< State is busy */
    Error     /*!< State is error */
} State;

template <typename U>
class ListOfValues
{
    public:
        QMap<U, QString> getList()
        {
            return list;
        }
        bool add(U val, QString label)
        {
            if (list.contains(val))
            {
                qDebug() << "ListOfValues - add - key " << val << " already exists (" << list[val] << ").";
                return false;
            }
            list[val] = label;
            return true;
        }
        bool del(U val)
        {
            if (!list.contains(val))
            {
                qDebug() << "ListOfValues - del - key " << val << " does not exist.";
                return false;
            }
            list.remove(val);
            return true;
        }
        bool clear()
        {
            list.clear();
            return true;
        }

    private:
        QMap<U, QString> list;
};

class ValueBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(ValueVisitor* pVisitor) = 0;
        virtual void accept(ValueVisitor* pVisitor, QVariantMap &data) = 0;

        ValueBase(const QString &label, const QString &order, const QString &hint);
        ~ValueBase();

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
        State state()
        {
            return mState;
        }
        void setState(const State &state);
        bool autoUpdate()
        {
            return mAutoUpdate;
        }
        void setAutoUpdate(const bool &v)
        {
            mAutoUpdate = v;
        }
    private:
        QString mLabel = "change me";
        QString mOrder = "change me";
        QString mHint = "";
        State mState = State::Idle;
        bool mAutoUpdate = false;
    signals:
        void valueChanged(OST::ValueBase*);


};

}

#endif

