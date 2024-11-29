#ifndef OST_ELEMENTFACTORY_H
#define OST_ELEMENTFACTORY_H

#include <baseclient.h>
#include <elementint.h>
#include <elementfloat.h>
#include <elementbool.h>
#include <elementstring.h>
#include <elementlight.h>
#include <elementimg.h>
#include <elementvideo.h>
#include <elementmessage.h>
#include <elementprg.h>
#include <elementdate.h>
#include <elementtime.h>

namespace  OST
{

class ElementFactory
{

    public:
        ElementFactory() = delete;
        static ElementBase* createElement(const QVariantMap &pData);
};

class ValueFactory
{

    public:
        ValueFactory() = delete;
        static ValueBase* createValue(ElementBase* &pElement);
};

}
#endif //OST_ELEMENTFACTORY_H
