#ifndef PROPERTYBASE_h_
#define PROPERTYBASE_h_

#include <propertyvisitor.h>
#include <elementbase.h>

namespace  OST
{

class PropertyVisitor;

typedef enum
{
    ReadOnly = 0,/*!< property is read only for frontend */
    WriteOnly,/*!< property is write only for frontend  */
    ReadWrite/*!< property is read write for frontend  */
} Permission;

/** @brief Converts integer to property permission
 *
 *  Defaults or error returns ReadOnly
 */
inline Permission IntToPermission(int val )
{
    if (val == 0) return ReadOnly;
    if (val == 1) return WriteOnly;
    if (val == 2) return ReadWrite;
    qDebug() << "Cant convert " << val << " to OST::Permission (0-2) - defaults to readonly";
    return ReadOnly;
}


/** @class OST::PropertyBase
 *  @brief Class to define common property
 *
 */
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

        /**
         * @brief Internal property's name
         * @return returns property's name
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        QString key()
        {
            return mKey;
        }
        /**
         * @brief Property's label
         * @return value of label
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        QString label()
        {
            return mLabel;
        }
        /**
         * @brief Property's permission
         * @return value of permission
         *
         * This value cannot be modified after instanciation
         * \hidecallgraph
         */
        Permission permission()
        {
            return mPermission;
        }
        /**
         * @brief Property's hierachy first level
         * @return value of level 1
         *
         * This value cannot be modified after instanciation
         *
         * It equivalent to indi "device" level
         * \hidecallgraph
         */
        QString level1()
        {
            return mLevel1;
        }
        /**
         * @brief Property's hierachy second level
         * @return value of level 2
         *
         * This value cannot be modified after instanciation
         *
         * It equivalent to indi "group" level
         * \hidecallgraph
         */
        QString level2()
        {
            return mLevel2;
        }
        /**
         * @brief Property's display order
         * @return order value
         *
         * This value cannot be modified after instanciation.
         *
         * This value sets display order value within same level1 / level2 values
         * \hidecallgraph
         */
        QString order()
        {
            return mOrder;
        }
        /**
         * @brief should this value be saved in profiles ?
         * @return true = yes, false = no
         *
         * This value cannot be modified after instanciation.
         * \hidecallgraph
         */
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
            emit propertyEvent("ap", key(), this);
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
        void eltChanged(OST::PropertyBase*);
        void valueSet(OST::PropertyBase*);
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

