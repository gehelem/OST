#include "propertybase.h"

PropertyBase::PropertyBase(const QString &label, const Permission &permission, const QString &level1, const QString &level2,
                           const QString &order, const bool &hasProfile, const bool &hasArray)
    : mLabel(label), mPermission(permission), mLevel1(level1), mLevel2(level2),
      mOrder(order), mHasProfile(hasProfile), mHasArray(hasArray)
{
    emit propertyCreated();
}
PropertyBase::~PropertyBase()
{

}
void PropertyBase::setState(State state)
{
    mState = state;
    emit stateChanged( state);
}

