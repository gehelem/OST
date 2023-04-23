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

class PropertyBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(PropertyVisitor* pVisitor) = 0;
        virtual void accept(PropertyVisitor* pVisitor, QVariantMap &data) = 0;

        PropertyBase(const QString &label, const Permission &permission, const QString &level1, const QString &level2,
                     const QString &order, const bool &hasProfile, const bool &hasArray
                    );
        ~PropertyBase();

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
        bool hasArray()
        {
            return mHasArray;
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


    signals:
        void stateChanged(OST::State);
        void propertyCreated(void);
        void valueChanged(OST::PropertyBase*);


    private:
        QString mLabel;
        Permission mPermission;
        QString mLevel1;
        QString mLevel2;
        QString mOrder;
        bool mHasProfile;
        bool mHasArray;
        State mState = State::Idle;
        bool mIsEnabled = true;


};

}
#endif

