#ifndef ELEMENTSINGLE_h
#define ELEMENTSINGLE_h

#include <elementbase.h>
#include <lovsingle.h>

namespace  OST
{

template <typename T>
class ElementSingle : public ElementBase
{

    public:

        ElementSingle(const QString &label, const QString &order, const QString &hint):
            ElementBase(label, order, hint) {}
        ~ElementSingle() {}
        QString getType() override
        {
            return "error";
        }
        QString getRealType()
        {
            return typeid(T).name();
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
        QString mGlobalLov = "";
        QString mPreIcon = "";
        QString mPostIcon = "";

};
template <typename T>
class ElementSingleNumeric : public ElementSingle<T>
{

    public:

        ElementSingleNumeric(const QString &label, const QString &order, const QString &hint):
            ElementSingle<T>(label, order, hint)
        {
        }
        ~ElementSingleNumeric<T>() {}
        T value()
        {
            return mValue;
        }
        bool setValue(const T &value, const bool &emitEvent)
        {
            if (mUseMinMax)
            {
                if (value < mMin)
                {
                    emit ElementSingle<T>::sendMessage(Warn,
                                                       "setValue - value too low " + QString::number(value) + " min= " + QString::number(mMin) );
                    return false;
                }
                if (value > mMax)
                {
                    emit ElementSingle<T>::sendMessage(Warn,
                                                       "setValue - value too high " + QString::number(value) + " max= " + QString::number(mMax) );
                    return false;
                }
            }
            mValue = value;
            if (emitEvent) emit ElementBase::valueSet(this);
            return true;
        }
        T min()
        {
            return mMin;
        }
        void setMin(const T &min)
        {
            mMin = min;
            emit ElementBase::eltChanged(this);
        }
        int max()
        {
            return mMax;
        }
        void setMax(const T &max)
        {
            mMax = max;
            emit ElementBase::eltChanged(this);
        }
        void setMinMax(const T &min, const T &max)
        {
            mMin = min;
            mMax = max;
            mUseMinMax = true;
            emit ElementBase::eltChanged(this);
        }
        void unSetMinMax(void)
        {
            mMin = 0;
            mMax = 0;
            mUseMinMax = false;
            emit ElementBase::eltChanged(this);
        }
        T step()
        {
            return mStep;
        }
        void setStep(const T &step)
        {
            mStep = step;
            emit ElementBase::eltChanged(this);
        }

        QString format()
        {
            return mFormat;
        }
        void setFormat(const QString &format)
        {
            mFormat = format;
            emit ElementBase::eltChanged(this);
        }
    private:
        T mValue;
        T mMin = 0;
        T mMax = 0;
        T mStep = 0;
        bool mUseMinMax = false;
        QString mFormat = "";

};
template <typename T>
class ElementSingleNotNumeric : public ElementSingle<T>
{

    public:

        ElementSingleNotNumeric(const QString &label, const QString &order, const QString &hint):
            ElementSingle<T>(label, order, hint)
        {
        }
        ~ElementSingleNotNumeric<T>() {}
        T value()
        {
            return mValue;
        }
        bool setValue(const T &value, const bool &emitEvent)
        {
            mValue = value;
            if (emitEvent) emit ElementBase::valueSet(this);
            return true;
        }
    private:
        T mValue;
};

template <typename T>
class ValueTemplate: public ValueBase
{
    public:
        ValueTemplate(ElementBase* element) : ValueBase(element) {}
        ~ValueTemplate() {}
        T value;
};

}
#endif
