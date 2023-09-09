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
    public slots:
        void OnSingleValueChanged(void)
        {
            emit valueChanged(this);
        }
    signals:
        void lovEvent();
        void gridEvent();
        void singleValueChanged();

};
template <typename T>
class ValueSingle : public ValueSingleSignalAndSlots
{

    public:

        ValueSingle(const QString &label, const QString &order, const QString &hint): ValueSingleSignalAndSlots(label, order, hint)
        {
            mLov.clear();
            mGridValues.clear();
            connect(this, &ValueSingle::singleValueChanged, this, &ValueSingleSignalAndSlots::OnSingleValueChanged);

        }
        ~ValueSingle() {}
        T value()
        {
            //if constexpr (std::is_same_v<T, int>)
            //{
            //    return 0;
            //}
            return mValue;
        }
        void setValue(const T &value, const bool &emitEvent)
        {
            mValue = value;
            if (emitEvent) emit singleValueChanged();
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
                emit sendMessage(Warn, "gridDel - trying to delete item " + QString::number(i) + " while grid size is " +
                                 mGridValues.size());
                return false;
            }
            mGridValues.removeAt(i);
            return true;
        }
        bool gridUpdate(const T &value, int i, const bool &emitEvent)
        {
            if (i >= mGridValues.size())
            {
                emit sendMessage(Warn, "gridUpdate - trying to update item " + QString::number(i) + " while grid size is " +
                                 mGridValues.size());
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
                QVariant v = val;
                emit sendMessage(Warn, "lovAdd - key " + v.toString() + " already exists (" + mLov[val] + ").");
                return false;
            }
            mLov[val] = label;
            return true;
        }
        bool lovUpdate(T val, QString label)
        {
            if (!mLov.contains(val))
            {
                QVariant v = val;
                emit sendMessage(Warn, "lovUpdate - key " + v.toString() + " doesn't exist.");
                return false;
            }
            mLov[val] = label;
            return true;
        }
        bool lovDel(T val)
        {
            if (!mLov.contains(val))
            {
                QVariant v = val;
                emit sendMessage(Warn, "lovDel - key " + v.toString() + " doesn't exist.");
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
        QString getRealType()
        {
            return typeid(T).name();
        }


    private:
        T mValue;
        QList<T> mGridValues = QList<T>();
        QMap<T, QString> mLov;


};

}
#endif
