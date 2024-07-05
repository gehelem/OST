#include "elementbase.h"
namespace  OST
{

ElementBase::ElementBase(const QString &label, const QString &order, const QString &hint)
    : mLabel(label), mOrder(order), mHint(hint)
{
}
ElementBase::~ElementBase()
{

}

QString ElementBase::label()
{
    return mLabel;
}
QString ElementBase::order()
{
    return mOrder;
}
QString ElementBase::hint()
{
    return mHint;
}
bool ElementBase::autoUpdate()
{
    return mAutoUpdate;
}
void ElementBase::setAutoUpdate(const bool &v)
{
    mAutoUpdate = v;
}
bool ElementBase::getDirectEdit()
{
    return mDirectEdit;
}
void ElementBase::setDirectEdit(const bool &v)
{
    mDirectEdit = v;
}
bool ElementBase::getBadge()
{
    return mBadge;
}
void ElementBase::setBadge(const bool &b)
{
    mBadge = b;
}
QString ElementBase::getPreIcon()
{
    return mPreIcon;
}
void ElementBase::setPreIcon(QString s)
{
    mPreIcon = s;
}
QString ElementBase::getPostIcon()
{
    return mPostIcon;
}
void ElementBase::setPostIcon(QString s)
{
    mPostIcon = s;
}
QString ElementBase::getGlobalLov()
{
    return mGlobalLov;
}
void ElementBase::setGlobalLov(QString lovName)
{
    mGlobalLov = lovName;
}

void ElementBase::OnLovChanged()
{
    emit lovChanged(this);
}

ValueBase::ValueBase(ElementBase *element): pElement(element)
{

}
ValueBase::~ValueBase()
{

}

}
