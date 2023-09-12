#ifndef VALUESINGLE_h
#define VALUESINGLE_h

#include <valuebase.h>
#include <lovsingle.h>

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

        ValueSingle(const QString &label, const QString &order, const QString &hint):
            ValueSingleSignalAndSlots(label, order, hint)
        {
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
        QString getType() override
        {
            return "error";
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
            if ( (mArrayLimit > 0) && (mGridValues.size() >= mArrayLimit)  )
            {
                mGridValues.removeFirst();
            }
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
        /*QMap<T, QString> getLov()
        {
            return mLov.getLov();
        }
        bool lovAdd(T val, QString label)
        {
            return mLov.lovAdd(val, label);
        }
        bool lovUpdate(T val, QString label)
        {
            return mLov.lovUpdate(val, label);
        }
        bool lovDel(T val)
        {
            return mLov.lovDel(val);
        }
        bool lovClear()
        {
            return mLov.lovClear();
        }*/
        QString getRealType()
        {
            return typeid(T).name();
        }
        int arrayLimit()
        {
            return mArrayLimit;
        }
        void setArrayLimit(int limit)
        {
            if (limit > 0 ) mArrayLimit = limit;
        }
    private:
        T mValue;
        QList<T> mGridValues = QList<T>();
        int mArrayLimit = 0;

};

}
#endif
