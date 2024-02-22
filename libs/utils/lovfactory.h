#ifndef OST_LOVFACTORY_H
#define OST_LOVFACTORY_H

#include <lovint.h>
#include <lovfloat.h>
#include <lovstring.h>
#include <lovlight.h>

namespace  OST
{

class LovFactory
{

    public:
        LovFactory() = delete;
        static LovBase* createLov(const QVariantMap &pData);
};

}
#endif //OST_LOVFACTORY_H
