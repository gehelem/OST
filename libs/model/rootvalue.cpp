#include "rootvalue.h"
namespace  OST
{

RootValue::RootValue(const QString &label)
    : mLabel(label)
{
    emit propertyCreated();
}
RootValue::~RootValue()
{

}
void RootValue::setState(State state)
{
    mState = state;
    emit stateChanged( state);
}
}
