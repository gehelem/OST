#include "propertytextdumper.h"
#include "valuetextdumper.h"
#include <sstream>
namespace  OST
{

std::string PropertyTextDumper::dumpPropertyCommons(PropertyBase *pProperty)
{
    std::stringstream stream;
    stream << " Label='" << pProperty->label().toStdString() << "'"
           << " Level1='" << pProperty->level1().toStdString() << "'"
           << " Level2='" << pProperty->level2().toStdString() << "'"
           << " Permission=" << pProperty->permission()
           << " State=" << pProperty->state()
           ;
    return stream.str();
}
void PropertyTextDumper::visit(PropertyMulti *pProperty)
{
    std::stringstream stream;
    stream << "Multi Property :" << dumpPropertyCommons(pProperty)
           << ". values=";

    foreach(const QString &key, pProperty->getValues().keys())
    {
        OST::ValueTextDumper d;

        pProperty->getValues()[key]->accept(&d);
        stream << " **** " << key.toStdString() << " : " << d.getResult().toStdString() << "//";
    }



    stream << " #####";
    mResult = QString::fromStdString(stream.str());
}

}
