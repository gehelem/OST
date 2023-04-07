#include "rootproperty.h"

RootProperty::RootProperty(const QString &label, const QString &level1, const QString &level2,
                           const Permission &permission )
    : RootValue(label), mLevel1(level1), mLevel2(level2), mPermission(permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&RootProperty::valueChanged,parent,QObject::)
}
RootProperty::~RootProperty()
{

}

