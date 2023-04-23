#include "valuetextdumper.h"
#include <sstream>
namespace  OST
{

std::string ValueTextDumper::dumpValueCommons(ValueBase *pValue)
{
    std::stringstream stream;
    stream << " Label=" << pValue->label().toStdString()
           << " State=" << pValue->state()
           ;
    return stream.str();
}
void ValueTextDumper::visit(ValueBool *pValue)
{
    std::stringstream stream;
    stream << "Bool Value :" << dumpValueCommons(pValue);
    if (pValue->value()) stream << "<ON>" ;
    else stream << "<OFF>";
    mResult = QString::fromStdString(stream.str());
}
void ValueTextDumper::visit(ValueInt *pValue)
{
    std::stringstream stream;
    stream << "Int Value :" << dumpValueCommons(pValue);
    stream << "Val = " << pValue->value();
    mResult = QString::fromStdString(stream.str());
}
void ValueTextDumper::visit(ValueFloat *pValue)
{
    std::stringstream stream;
    stream << "Float Value :" << dumpValueCommons(pValue);
    stream << "Val = " << pValue->value();
    mResult = QString::fromStdString(stream.str());
}
void ValueTextDumper::visit(ValueString *pValue)
{
    std::stringstream stream;
    stream << "String Value :" << dumpValueCommons(pValue);
    stream << "Val = " << pValue->value().toStdString();
    mResult = QString::fromStdString(stream.str());
}
void ValueTextDumper::visit(ValueLight *pValue)
{
    std::stringstream stream;
    stream << "Light Value :" << dumpValueCommons(pValue);
    switch ( pValue->state())
    {
        case Idle:
            stream << "Val = Idle";
            break;
        case Ok:
            stream << "Val = Ok";
            break;
        case Busy:
            stream << "Val = Busy";
            break;
        case Error:
            stream << "Val = Error";
            break;
        default:
            stream << "Val = Unknown";
    }
    mResult = QString::fromStdString(stream.str());
}

}
