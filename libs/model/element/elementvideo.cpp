#include "valuevideo.h"
namespace  OST
{
ValueVideo::ValueVideo(const QString &label, const QString &order, const QString &hint)
    : ValueSingleNotNumeric(label, order, hint)
{
}
ValueVideo::~ValueVideo()
{
}

}
