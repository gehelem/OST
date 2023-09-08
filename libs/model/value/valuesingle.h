#ifndef VALUESINGLE_h
#define VALUESINGLE_h

#include <valuebase.h>

namespace  OST
{

class ValueSingleSignalAndSlots : public ValueBase
{
        Q_OBJECT
    public:
        ValueSingleSignalAndSlots(const QString &label, const QString &order, const QString &hint) : ValueBase(label, order, hint)
        {

        }
        ~ValueSingleSignalAndSlots() {}
    signals:
        void lovEvent();
        void gridEvent();
};

template <class T>
class ValueSingle : public ValueSingleSignalAndSlots
{

    public:

        ValueSingle(const QString &label, const QString &order, const QString &hint): ValueSingleSignalAndSlots(label, order, hint)
        {

        }
        ~ValueSingle() {}
        T value()
        {
            return mValue;
        }
        void setValue(const T &value, const bool &emitEvent)
        {
            mValue = value;
            if (emitEvent) emit valueChanged(this);
        }
        QList<T> getGrid()
        {
            return mGridValues;
        }
        bool gridAdd(T val)
        {
            mGridValues.append(val);
            return true;
        }
        bool gridDel(int i)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - del - trying to delete item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues.removeAt(i);
            return true;
        }
        bool gridUpdate(const T &value, int i, const bool &emitEvent)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - update - trying to update item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues[i] = value;
            if (emitEvent) emit listChanged(this);
            return true;
        }
        bool gridClear()
        {
            mGridValues.clear();
            return true;
        }
        void setGrid(QList<T> vals )
        {
            mGridValues.clear();
            mGridValues = vals;
        }
        QMap<T, QString> getLov()
        {
            return mLov;
        }
        bool lovAdd(T val, QString label)
        {
            if (mLov.contains(val))
            {
                qDebug() << "ListOfValues - add - key " << val << " already exists (" << mLov[val] << ").";
                return false;
            }
            mLov[val] = label;
            return true;
        }
        bool lovUpdate(T val, QString label)
        {
            if (!mLov.contains(val))
            {
                qDebug() << "ListOfValues - update - key " << val << " doesn't exist (" << mLov[val] << ").";
                return false;
            }
            mLov[val] = label;
            return true;
        }
        bool lovDel(T val)
        {
            if (!mLov.contains(val))
            {
                qDebug() << "ListOfValues - del - key " << val << " does not exist.";
                return false;
            }
            mLov.remove(val);
            return true;
        }
        bool lovClear()
        {
            mLov.clear();
            return true;
        }


        QString getType2()
        {
            return typeid(T).name();
            emit lovEvent();
        }

    private:
        T mValue;
        QList<T> mGridValues;
        QMap<T, QString> mLov;



};

}
#endif
