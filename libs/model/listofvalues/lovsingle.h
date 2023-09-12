#ifndef LOVSINGLE_h
#define LOVSINGLE_h

#include <lovbase.h>

namespace  OST
{

template <typename T>
class LovSingle : public LovBase
{

    public:

        LovSingle(const QString &label): LovBase(label)
        {
            mLov.clear();
        }
        ~LovSingle() {}
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
        QMap<T, QString> mLov;


};

}
#endif
