#include "rootproperty.h"

RootProperty::RootProperty(const QString &label, const QString &level1, const QString &level2,
                           const Permission &permission )
    : mLabel(label), mLevel1(level1), mLevel2(level2), mPermission(permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&RootProperty::valueChanged,parent,QObject::)
}
RootProperty::~RootProperty()
{

}
void RootProperty::setState(State state)
{
    mState = state;
    emit stateChanged( state);
}
RootProperty::State RootProperty::state()
{
    return mState;
}
