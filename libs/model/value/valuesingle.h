#ifndef VALUESINGLE_h
#define VALUESINGLE_h

#include <valuebase.h>
#include <lovsingle.h>

namespace  OST
{

template <typename T>
class ValueSingle : public ValueBase
{

    public:

        ValueSingle(const QString &label, const QString &order, const QString &hint):
            ValueBase(label, order, hint)
        {
            mGridValues.clear();
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
            if (emitEvent) emit valueChanged(this);
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
        QString getGlobalLov()
        {
            return mGlobalLov;
        }
        void setGlobalLov(QString lovName)
        {
            mGlobalLov = lovName;
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
    private:
        T mValue;
        QList<T> mGridValues = QList<T>();
        int mArrayLimit = 0;
        QString mGlobalLov = "";
        QString mPreIcon = "";
        QString mPostIcon = "";

};
template <typename T>
class ValueSingleNumeric : public ValueSingle<T>
{

    public:

        ValueSingleNumeric(const QString &label, const QString &order, const QString &hint):
            ValueSingle<T>(label, order, hint)
        {
        }
        ~ValueSingleNumeric<T>() {}
        void setValue(const T &value, const bool &emitEvent)
        {
            if (mUseMinMax)
            {
                if (value < mMin)
                {
                    emit ValueSingle<T>::sendMessage(Warn,
                                                     "valueUpdate - value too low " + QString::number(value) + " min= " + QString::number(mMin) );
                    return;
                }
                if (value > mMax)
                {
                    emit ValueSingle<T>::sendMessage(Warn,
                                                     "valueUpdate - value too high " + QString::number(value) + " max= " + QString::number(mMax) );
                    return;
                }
            }
            ValueSingle<T>::setValue(value, emitEvent);
        }
        T min()
        {
            return mMin;
        }
        void setMin(const T &min)
        {
            mMin = min;
        }
        int max()
        {
            return mMax;
        }
        void setMax(const T &max)
        {
            mMax = max;
        }
        void setMinMax(const T &min, const T &max)
        {
            mMin = min;
            mMax = max;
            mUseMinMax = true;
        }
        void unSetMinMax(void)
        {
            mMin = 0;
            mMax = 0;
            mUseMinMax = false;
        }
        T step()
        {
            return mStep;
        }
        void setStep(const T &step)
        {
            mStep = step;
        }

        QString format()
        {
            return mFormat;
        }
        void setFormat(const QString &format)
        {
            mFormat = format;
        }
    private:
        T mMin = 0;
        T mMax = 0;
        T mStep = 0;
        bool mUseMinMax = false;
        QString mFormat = "";

};

}
#endif
