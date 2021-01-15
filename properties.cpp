#include "properties.h"

OstProperties::OstProperties(void)
{
}
OstProperties::~OstProperties()
{
}
QString OstProperties::getText(char const *name)
{
    return property(name).toString();
}
bool OstProperties::setText(char const *name,QString text)
{
    return setProperty(name,text);
}
double OstProperties::getNumber(char const *name)
{
    return property(name).toDouble();
}
bool OstProperties::setNumber(char const *name,double num)
{
    return setProperty(name,num);
}
