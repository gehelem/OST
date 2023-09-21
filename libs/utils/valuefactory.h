#ifndef OST_VALUEFACTORY_H
#define OST_VALUEFACTORY_H

#include <baseclient.h>
#include <valueint.h>
#include <valuefloat.h>
#include <valuebool.h>
#include <valuestring.h>
#include <valuelight.h>
#include <valueimg.h>
#include <valuevideo.h>
#include <valuemessage.h>
#include <valuegraph.h>

namespace  OST
{

class ValueFactory
{

    public:
        ValueFactory() = delete;
        static ValueBase* createValue(const QVariantMap &pData);
};

}
#endif //OST_VALUEFACTORY_H
