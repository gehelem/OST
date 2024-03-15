#include "valuejsondumper.h"
#include "elementbool.h"
#include "elementfloat.h"
#include "elementimg.h"
#include "elementint.h"
#include "elementlight.h"
#include "elementstring.h"
#include "elementmessage.h"
#include "elementgraph.h"
#include "elementvideo.h"
#include "elementprg.h"
namespace  OST
{

void ValueJsonDumper::visit(ValueBool * pValue)
{
    mResult = pValue->value;
};
void ValueJsonDumper::visit(ValueFloat * pValue)
{
    mResult = pValue->value;
};
void ValueJsonDumper::visit(ValueInt * pValue)
{
    mResult = pValue->value;
};
void ValueJsonDumper::visit(ValueString * pValue)
{
    mResult = pValue->value;
};
void ValueJsonDumper::visit(ValueLight * pValue)
{
    mResult = pValue->value;
};
void ValueJsonDumper::visit(ValueImg *  pValue)
{
    //mResult = pValue->value;
    mResult = "to be implementted";
};
void ValueJsonDumper::visit(ValueVideo *  pValue)
{
    //mResult = pValue->value;
    mResult = "to be implementted";
};
void ValueJsonDumper::visit(ValueMessage *  pValue)
{
    QJsonObject ob;
    ob["level"] = pValue->value.level;
    ob["message"] = pValue->value.message;
    ob["ts"] = pValue->value.ts.toString("yyyy/MM/dd hh:mm:ss.zzz");;
    mResult = ob;
};
void ValueJsonDumper::visit(ValueGraph *  pValue)
{
    //mResult = pValue->value;
    mResult = "to be implementted";
};
void ValueJsonDumper::visit(ValuePrg * pValue)
{
    mResult = pValue->value;
};


}
