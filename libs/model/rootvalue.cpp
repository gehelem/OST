#include "rootvalue.h"

RootValue::RootValue(const QString &label)
    : mLabel(label)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&RootProperty::valueChanged,parent,QObject::)
}
RootValue::~RootValue()
{

}
void RootValue::setState(State state)
{
    mState = state;
    emit stateChanged( state);
}

