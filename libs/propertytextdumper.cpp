#include "propertytextdumper.h"
#include <sstream>
#include <basicproperty.h>
#include <numberproperty.h>
#include <textproperty.h>

std::string PropertyTextDumper::dumpPropertyCommons(RootProperty *pProperty)
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
void PropertyTextDumper::visit(BasicProperty *pProperty)
{
    std::stringstream stream;
    stream << "Basic Property :" << dumpPropertyCommons(pProperty);
    _result = QString::fromStdString(stream.str());
}
void PropertyTextDumper::visit(TextProperty *pProperty)
{
    std::stringstream stream;
    stream << "Text Property :" << dumpPropertyCommons(pProperty)
           << ". Value=" << pProperty->value().toStdString();
    _result = QString::fromStdString(stream.str());
}
void PropertyTextDumper::visit(NumberProperty *pProperty)
{
    std::stringstream stream;
    stream << "Number Property :" << dumpPropertyCommons(pProperty)
           << ". Value=" << pProperty->value();
    _result = QString::fromStdString(stream.str());
}


