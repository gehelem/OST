#include "numberproperty.h"

NumberProperty::NumberProperty(const QString &label, const QString &level1, const QString &level2,
                               const Permission &permission )
    : RootProperty(label, level1, level2, permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&BasicProperty::valueChanged,parent,QObject::)
}
NumberProperty::~NumberProperty()
{

}
void NumberProperty::setValue(double value)
{
    mValue = value;
    emit valueChanged(value);
}
