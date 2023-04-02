#include "basicproperty.h"

BasicProperty::BasicProperty(const QString &label, const QString &level1, const QString &level2,
                             const Permission &permission )
    : mLabel(label), mLevel1(level1), mLevel2(level2), mPermission(permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&BasicProperty::valueChanged,parent,QObject::)
}
BasicProperty::~BasicProperty()
{

}
void BasicProperty::setState(State state)
{
    mState = state;
    emit stateChanged( state);
}
BasicProperty::State BasicProperty::state() const
{
    return mState;
}
