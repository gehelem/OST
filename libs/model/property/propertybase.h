#ifndef PROPERTYBASE_h_
#define PROPERTYBASE_h_

#include <propertyvisitor.h>
#include <valuebase.h>

namespace  OST
{

class PropertyVisitor;

typedef enum
{
    ReadOnly = 0,
    WriteOnly,
    ReadWrite
} Permission;
inline Permission IntToPermission(int val )
{
    if (val == 0) return ReadOnly;
    if (val == 1) return WriteOnly;
    if (val == 2) return ReadWrite;
    qDebug() << "Cant convert " << val << " to OST::Permission (0-2) - defaults to readonly";
    return ReadOnly;
}
class PropertyBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(PropertyVisitor* pVisitor) = 0;
        virtual void accept(PropertyVisitor* pVisitor, QVariantMap &data) = 0;

        PropertyBase(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                     const QString &level2,
                     const QString &order, const bool &hasProfile, const bool &hasArray
                    );
        ~PropertyBase();

        QString key()
        {
            return mKey;
        }
        QString label()
        {
            return mLabel;
        }
        Permission permission()
        {
            return mPermission;
        }
        QString level1()
        {
            return mLevel1;
        }
        QString level2()
        {
            return mLevel2;
        }
        QString order()
        {
            return mOrder;
        }
        bool hasProfile()
        {
            return mHasProfile;
        }
        State state()
        {
            return mState;
        }
        void setState(State state);
        bool isEnabled()
        {
            return mIsEnabled;
        }
        void enable(void)
        {
            mIsEnabled = true;
        }
        void disable(void)
        {
            mIsEnabled = false;
        }
        bool hasArray()
        {
            return mHasArray;
        }
        void setHasArray(bool hasarray);
        bool getShowArray()
        {
            return mShowArray;
        }
        void setShowArray(bool b)
        {
            mShowArray = b;
        }
        int getArrayLimit()
        {
            return mArrayLimit;
        }
        void setArrayLimit(int limit)
        {
            if (limit > 0) mArrayLimit = limit;
        }
        bool getBadge()
        {
            return mBadge;
        }
        void setBadge(bool b)
        {
            mBadge = b;
        }
        void sendInfo(QString m)
        {
            emit sendMessage(Info, key() + "-" + m);
        }
        void sendWarning(QString m)
        {
            emit sendMessage(Warn, key() + "-" + m);
        }
        void sendError(QString m)
        {
            emit sendMessage(Err, key() + "-" + m);
        }
    signals:
        void stateChanged(OST::State);
        void propertyCreated(void);
        void valueChanged(OST::PropertyBase*);
        void propertyEvent(QString, QString, OST::PropertyBase*);
        void sendMessage(MsgLevel, QString);

    private:
        QString mKey = "";
        QString mLabel = "change me";
        Permission mPermission = Permission::ReadOnly;
        QString mLevel1 = "";
        QString mLevel2 = "";
        QString mOrder = 0;
        bool mHasProfile = false;
        State mState = State::Idle;
        bool mIsEnabled = true;
        bool mHasArray = false;
        bool mShowArray = true;
        int mArrayLimit = 0;
        bool mBadge = false;



};

}
#endif

