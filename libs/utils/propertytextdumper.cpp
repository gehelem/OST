#include "propertytextdumper.h"
#include <sstream>
namespace  OST
{

std::string PropertyTextDumper::dumpPropertyCommons(PropertyBase *pProperty)
{
    std::stringstream stream;
    stream << " Label=" << pProperty->label().toStdString()
           << " Level1=" << pProperty->level1().toStdString()
           << " Level2=" << pProperty->level2().toStdString()
           << " Permission=" << pProperty->permission()
           << " State=" << pProperty->state()
           ;
    return stream.str();
}
void PropertyTextDumper::visit(PropertySimple *pProperty)
{
    std::stringstream stream;
    stream << "Simple Property :" << dumpPropertyCommons(pProperty);
    _result = QString::fromStdString(stream.str());
}
void PropertyTextDumper::visit(PropertyMulti *pProperty)
{
    std::stringstream stream;
    stream << "Multi Property :" << dumpPropertyCommons(pProperty)
           << ". elements=";

    QList<ValueBase*>::iterator i;
    for( int i = 0; i < pProperty->getValues().count(); ++i )
    {

    }


    foreach(const QString &key, pProperty->getValues())
    {
        PropertyTextDumper d;
        pProperty->getElts()[key]->accept(&d);
        stream << " **** key : " << key.toStdString() << " / content : " << d.getResult().toStdString();

    }
    stream << " #####";
    _result = QString::fromStdString(stream.str());
}

}
