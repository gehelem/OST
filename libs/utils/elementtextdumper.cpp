#include "elementtextdumper.h"
#include <sstream>
namespace  OST
{

std::string ElementTextDumper::dumpElementCommons(ElementBase *pElement)
{
    std::stringstream stream;
    stream << " Label='" << pElement->label().toStdString() << "'"
           ;
    return stream.str();
}
void ElementTextDumper::visit(ElementBool *pElement)
{
    std::stringstream stream;
    stream << "Bool Value :" << dumpElementCommons(pElement);
    if (pElement->value()) stream << "Val = <ON> " ;
    else stream << "Val = <OFF> ";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementInt *pElement)
{
    std::stringstream stream;
    stream << "Int Value :" << dumpElementCommons(pElement);
    stream << "Val = " << pElement->value() << " ";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementFloat *pElement)
{
    std::stringstream stream;
    stream << "Float Value :" << dumpElementCommons(pElement);
    stream << "Val = " << pElement->value() << " ";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementString *pElement)
{
    std::stringstream stream;
    stream << "String Value :" << dumpElementCommons(pElement);
    stream << "Val = '" << pElement->value().toStdString() << "'";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementLight *pElement)
{
    std::stringstream stream;
    stream << "Light Value :" << dumpElementCommons(pElement);
    switch ( pElement->value())
    {
        case Idle:
            stream << "Val = Idle ";
            break;
        case Ok:
            stream << "Val = Ok ";
            break;
        case Busy:
            stream << "Val = Busy ";
            break;
        case Error:
            stream << "Val = Error ";
            break;
        default:
            stream << "Val = Unknown ";
    }
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementImg *pElement)
{
    std::stringstream stream;
    stream << "Image Value :" << dumpElementCommons(pElement);
    //stream << "Val = '" << pElement->value().toStdString() << "'";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementVideo *pElement)
{
    std::stringstream stream;
    stream << "Video Value :" << dumpElementCommons(pElement);
    stream << "Url = '" << pElement->value().url.toStdString() << "'";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementMessage *pElement)
{
    std::stringstream stream;
    stream << "Message Value :" << dumpElementCommons(pElement);
    //stream << "Val = '" << pElement->value().toStdString() << "'";
    mResult = QString::fromStdString(stream.str());
}
void ElementTextDumper::visit(ElementPrg *pElement)
{
    std::stringstream stream;
    stream << "Progress Value :" << dumpElementCommons(pElement);
    //stream << "Val = '" << pElement->value().toStdString() << "'";
    mResult = QString::fromStdString(stream.str());
}

}
