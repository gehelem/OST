#include "valuejsondumper.h"
#include "elementbool.h"
#include "elementfloat.h"
#include "elementimg.h"
#include "elementint.h"
#include "elementlight.h"
#include "elementstring.h"
#include "elementmessage.h"
#include "elementvideo.h"
#include "elementprg.h"
#include "elementdate.h"
#include "elementtime.h"
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
    QJsonObject imgdata;
    imgdata["urljpeg"] =  pValue->value.mUrlJpeg;
    imgdata["urlfits"] = pValue->value.mUrlFits;
    imgdata["urlthumbnail"] = pValue->value.mUrlThumbnail;
    imgdata["urloverlay"] = pValue->value.mUrlOverlay;
    imgdata["channels"] = pValue->value.channels;
    imgdata["width"] = pValue->value.width;
    imgdata["height"] = pValue->value.height;
    imgdata["snr"] = pValue->value.SNR;
    imgdata["hfravg"] = pValue->value.HFRavg;
    imgdata["stars"] = pValue->value.starsCount;
    imgdata["issolved"] = pValue->value.isSolved;
    imgdata["solverra"] = pValue->value.solverRA;
    imgdata["solverde"] = pValue->value.solverDE;


    QJsonArray arr;
    arr = QJsonArray();
    for (int i = 0; i < pValue->value.channels; i++)
    {
        arr.append(pValue->value.min[i]);
    }
    imgdata["min"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pValue->value.channels; i++)
    {
        arr.append(pValue->value.max[i]);
    }
    imgdata["max"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pValue->value.channels; i++)
    {
        arr.append(pValue->value.mean[i]);
    }
    imgdata["mean"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pValue->value.channels; i++)
    {
        arr.append(pValue->value.median[i]);
    }
    imgdata["median"] = arr;

    arr = QJsonArray();
    for (int i = 0; i < pValue->value.channels; i++)
    {
        arr.append(pValue->value.stddev[i]);
    }
    imgdata["stddev"] = arr;

    arr = QJsonArray();

    for (int i = 0; i <  pValue->value.channels; i++  )
    {
        QJsonArray oneChannel = QJsonArray();
        QJsonArray freq = QJsonArray();
        for (int j = 0; j < pValue->value.histogram[0].size(); j++  )
        {
            freq = QJsonArray();
            freq.append(pValue->value.histogram[i][j]);
            oneChannel.append(freq);
        }
        arr.append(oneChannel);
    }
    imgdata["histogram"] = arr;

    mResult = imgdata;




};
void ValueJsonDumper::visit(ValueVideo *  pValue)
{
    Q_UNUSED(pValue)
    //mResult = pValue->value;
    mResult = "to be implemented";
};
void ValueJsonDumper::visit(ValueMessage *  pValue)
{
    QJsonObject ob;
    ob["level"] = pValue->value.level;
    ob["message"] = pValue->value.message;
    ob["ts"] = pValue->value.ts.toString("yyyy/MM/dd hh:mm:ss.zzz");;
    mResult = ob;
};
void ValueJsonDumper::visit(ValuePrg * pValue)
{
    QJsonObject ob;
    ob["value"] = pValue->value.value;
    ob["dynlabel"] = pValue->value.dynlabel;
    mResult = ob;

};
void ValueJsonDumper::visit(ValueDate * pValue)
{
    QJsonObject ob;
    ob["year"] = pValue->value.year;
    ob["month"] = pValue->value.month;
    ob["day"] = pValue->value.day;
    mResult = ob;

};
void ValueJsonDumper::visit(ValueTime * pValue)
{
    QJsonObject ob;
    ob["hh"] = pValue->value.hh;
    ob["mm"] = pValue->value.mm;
    ob["ss"] = pValue->value.ss;
    mResult = ob;

};


}
