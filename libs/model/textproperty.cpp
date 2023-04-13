#include "textproperty.h"

TextProperty::TextProperty(const QString &label, const QString &level1, const QString &level2,
                           const Permission &permission )
    : RootProperty(label, level1, level2, permission)
{
    //emit propertyCreated(this->metaObject());
    //parent->connect(this,&BasicProperty::valueChanged,parent,QObject::)
}
TextProperty::~TextProperty()
{

}
void TextProperty::setValue(QString value)
{
    mValue = value;
    emit valueChanged(this);
}
