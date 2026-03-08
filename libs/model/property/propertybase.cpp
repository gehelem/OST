#include "propertybase.h"
namespace  OST
{

PropertyBase::PropertyBase(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                           const QString &level2,
                           const QString &order, const bool &hasProfile)
    : mKey(key), mLabel(label), mPermission(permission), mLevel1(level1), mLevel2(level2),
      mOrder(order), mHasProfile(hasProfile)
{
    qRegisterMetaType<OST::EvType>("OST::EvType");
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    if (order == "") emit logMessage(OST::LogLevel::Warning, "PropertyBase %1 %2 order ko (%3/%4)", {label, key, level1, level2});
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
void PropertyBase::setState(State state, bool emitEvent)
{
    mState = state;
    if (emitEvent) emit PropertyBase::prpEvent(OST::EvType::ps, QVariant(), nullptr, this);
}
bool PropertyBase::isEnabled()
{
    return mIsEnabled;
}
void PropertyBase::enable(void)
{
    mIsEnabled = true;
    emit PropertyBase::prpEvent(OST::EvType::ps, QVariant(), nullptr, this);
}
void PropertyBase::disable(void)
{
    mIsEnabled = false;
    emit PropertyBase::prpEvent(OST::EvType::ps, QVariant(), nullptr, this);
}
bool PropertyBase::getBadge()
{
    return mBadge;
}
void PropertyBase::setBadge(bool b)
{
    mBadge = b;
    emit PropertyBase::prpEvent(OST::EvType::aa, QVariant(), nullptr, this);

}
void PropertyBase::logDebug(QString m, const QVariantList &args)
{
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(LogLevel::Debug, newMessage, newArgs);
}
void PropertyBase::logInfo(QString m, const QVariantList &args)
{
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(LogLevel::Info, newMessage, newArgs);
}
void PropertyBase::logWarning(QString m, const QVariantList &args)
{
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(LogLevel::Warning, newMessage, newArgs);
}
void PropertyBase::logError(QString m, const QVariantList &args)
{
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(LogLevel::Error, newMessage, newArgs);
}
void PropertyBase::logCritical(QString m, const QVariantList &args)
{
    QVariantList newArgs = args;
    newArgs.prepend(key());
    QString newMessage = "%1 - " + incrementPlaceholders(m);
    emit logMessage(LogLevel::Critical, newMessage, newArgs);
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
bool PropertyBase::autoUpdate()
{
    return mAutoUpdate;
}
void PropertyBase::setAutoUpdate(bool b)
{
    mAutoUpdate = b;
}

}
