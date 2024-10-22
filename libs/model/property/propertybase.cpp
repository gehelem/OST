#include "propertybase.h"
namespace  OST
{

PropertyBase::PropertyBase(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                           const QString &level2,
                           const QString &order, const bool &hasProfile)
    : mKey(key), mLabel(label), mPermission(permission), mLevel1(level1), mLevel2(level2),
      mOrder(order), mHasProfile(hasProfile)
{
    qRegisterMetaType<OST::MsgLevel>("MsgLevel");
    if (order == "") qDebug() << "*** PropertyBase order ko *** "  << label;
}
PropertyBase::~PropertyBase()
{
}

QString PropertyBase::key()
{
    return mKey;
}
QString PropertyBase::label()
{
    return mLabel;
}
Permission PropertyBase::permission()
{
    return mPermission;
}
QString PropertyBase::level1()
{
    return mLevel1;
}
QString PropertyBase::level2()
{
    return mLevel2;
}
QString PropertyBase::order()
{
    return mOrder;
}
bool PropertyBase::hasProfile()
{
    return mHasProfile;
}
State PropertyBase::state()
{
    return mState;
}
void PropertyBase::setState(State state)
{
    mState = state;
    emit propertyEvent("ap", key(), this);

}
bool PropertyBase::isEnabled()
{
    return mIsEnabled;
}
void PropertyBase::enable(void)
{
    mIsEnabled = true;
    emit propertyEvent("ap", key(), this);
}
void PropertyBase::disable(void)
{
    mIsEnabled = false;
    emit propertyEvent("ap", key(), this);
}
bool PropertyBase::getBadge()
{
    return mBadge;
}
void PropertyBase::setBadge(bool b)
{
    mBadge = b;
    emit propertyEvent("ap", key(), this);
}
void PropertyBase::sendInfo(QString m)
{
    emit sendMessage(Info, key() + "-" + m);
}
void PropertyBase::sendWarning(QString m)
{
    emit sendMessage(Warn, key() + "-" + m);
}
void PropertyBase::sendError(QString m)
{
    emit sendMessage(Err, key() + "-" + m);
}
QString PropertyBase::getPreIcon1(void)
{
    return mPreIcon1;
}
void PropertyBase::setPreIcon1(QString s)
{
    mPreIcon1 = s;
}
QString PropertyBase::getPreIcon2(void)
{
    return mPreIcon2;
}
void PropertyBase::setPreIcon2(QString s)
{
    mPreIcon2 = s;
}
QString PropertyBase::getPostIcon1(void)
{
    return mPostIcon1;
}
void PropertyBase::setPostIcon1(QString s)
{
    mPostIcon1 = s;
}
QString PropertyBase::getPostIcon2(void)
{
    return mPostIcon2;
}
void PropertyBase::setPostIcon2(QString s)
{
    mPostIcon2 = s;
}
QString PropertyBase::getFreeValue(void)
{
    return mFreeValue;
}
void PropertyBase::setFreeValue(QString s)
{
    mFreeValue = s;
}

}
