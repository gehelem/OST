#include "multiproperty.h"
namespace  OST
{

MultiProperty::MultiProperty(const QString &label, const QString &level1, const QString &level2,
                             const Permission &permission )
    : RootMulti(label, level1, level2, permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&BasicProperty::valueChanged,parent,QObject::)
}
MultiProperty::~MultiProperty()
{

}
}
