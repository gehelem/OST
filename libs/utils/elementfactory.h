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
#include <elementgraph.h>
#include <elementprg.h>

namespace  OST
{

class ElementFactory
{

    public:
        ElementFactory() = delete;
        static ElementBase* createElement(const QVariantMap &pData);
};

class GridFactory
{

    public:
        GridFactory() = delete;
        static GridBase* createGrid(ElementBase* &pValue);
};

class ValueFactory
{

    public:
        ValueFactory() = delete;
        static ValueBase* createValue(ElementBase* &pElement);
};

}
#endif //OST_ELEMENTFACTORY_H
