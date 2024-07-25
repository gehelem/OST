#ifndef ELEMENTTEMPLATE_h
#define ELEMENTTEMPLATE_h

#include <elementbase.h>
#include <lovsingle.h>

namespace  OST
{

template <typename T>
class ElementTemplate: public ElementBase
{

    public:

        ElementTemplate(const QString &label, const QString &order, const QString &hint):
            ElementBase(label, order, hint) {}
        ~ElementTemplate() {}
        T value()
        {
            return mValue;
        }
        QString getType() override
        {
            return "error";
        }
        QString getRealType()
        {
            return typeid(T).name();
        }
    protected:
        T mValue;
};
template <typename T>
class ElementTemplateNumeric : public ElementTemplate<T>
{

    public:

        ElementTemplateNumeric(const QString &label, const QString &order, const QString &hint):
            ElementTemplate<T>(label, order, hint)
        {
        }
        ~ElementTemplateNumeric<T>() {}
        bool setValue(const T &value, const bool &emitEvent)
        {
            if (mUseMinMax)
            {
                if (value < mMin)
                {
                    emit ElementTemplate<T>::sendMessage(Warn,
                                                         "setValue - value too low " + QString::number(value) + " min= " + QString::number(mMin) );
                    return false;
                }
                if (value > mMax)
                {
                    emit ElementTemplate<T>::sendMessage(Warn,
                                                         "setValue - value too high " + QString::number(value) + " max= " + QString::number(mMax) );
                    return false;
                }
            }
            ElementTemplate<T>::mValue = value;
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
        SliderRule slider()
        {
            return mSlider;
        }
        void setSlider(const SliderRule &s)
        {
            mSlider = s;
            emit ElementBase::eltChanged(this);
        }

    private:
        T mMin = 0;
        T mMax = 0;
        T mStep = 0;
        bool mUseMinMax = false;
        QString mFormat = "";
        SliderRule mSlider = SliderNone;
};
template <typename T>
class ElementTemplateNotNumeric : public ElementTemplate<T>
{

    public:

        ElementTemplateNotNumeric(const QString &label, const QString &order, const QString &hint):
            ElementTemplate<T>(label, order, hint)
        {
        }
        ~ElementTemplateNotNumeric<T>() {}
        bool setValue(const T &value, const bool &emitEvent)
        {
            ElementTemplate<T>::mValue = value;
            if (emitEvent) emit ElementBase::valueSet(this);
            return true;
        }

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
