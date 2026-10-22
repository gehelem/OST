#ifndef LOVSINGLE_h
#define LOVSINGLE_h

#include <lovbase.h>
#include <QMutex>
#include <QMutexLocker>

namespace  OST
{

// mLov is read from LovJsonDumper::visit() on whatever thread delivers a
// queued lovChanged (typically the Qt main thread), while it can be
// written from an INDI client callback running on that connection's own
// socket thread (IndiModule inherits INDI::BaseClient directly, whose
// callbacks aren't marshaled to any particular thread) - mMutex keeps
// those accesses from touching the underlying QMap at the same time.
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
            QMutexLocker locker(&mMutex);
            return mLov;
        }
        bool lovAdd(T val, QString label)
        {
            bool alreadyExists;
            QString existing;
            {
                QMutexLocker locker(&mMutex);
                alreadyExists = mLov.contains(val);
                if (alreadyExists)
                    existing = mLov[val];
                else
                    mLov[val] = label;
            }
            if (alreadyExists)
            {
                QVariant v = val;
                emit sendMessage(LogLevel::Warning, "lovAdd - key %1 already exists (%2)", {v.toString(), existing});
                return false;
            }
            emit lovChanged(this);
            return true;
        }
        bool lovUpdate(T val, QString label)
        {
            bool found;
            {
                QMutexLocker locker(&mMutex);
                found = mLov.contains(val);
                if (found)
                    mLov[val] = label;
            }
            if (!found)
            {
                QVariant v = val;
                emit sendMessage(LogLevel::Warning, "lovUpdate - key %1 doesn't exist", {v.toString()});
                return false;
            }
            emit lovChanged(this);
            return true;
        }
        bool lovDel(T val)
        {
            bool found;
            {
                QMutexLocker locker(&mMutex);
                found = mLov.contains(val);
                if (found)
                    mLov.remove(val);
            }
            if (!found)
            {
                QVariant v = val;
                emit sendMessage(LogLevel::Warning, "lovDel - key %1 doesn't exist", {v.toString()});
                return false;
            }
            emit lovChanged(this);
            return true;
        }
        bool lovClear()
        {
            {
                QMutexLocker locker(&mMutex);
                mLov.clear();
            }
            emit lovChanged(this);
            return true;
        }
        QString getRealType()
        {
            return typeid(T).name();
        }
        bool contains(T key)
        {
            QMutexLocker locker(&mMutex);
            return mLov.contains(key);
        }
    private:
        QMap<T, QString> mLov;
        QMutex mMutex;

};

}
#endif
