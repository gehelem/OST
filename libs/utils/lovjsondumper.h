#ifndef OST_LOVJSONDUMPER_H
#define OST_LOVJSONDUMPER_H

#include <string>
#include <lovstring.h>
#include <lovint.h>
#include <lovfloat.h>
#include <lovvisitor.h>
namespace  OST
{

class LovJsonDumper : public LovVisitor
{

    public:
        LovJsonDumper() = default;

        void visit(LovString *pLov) override;
        void visit(LovInt *pLov) override;
        void visit(LovFloat *pLov) override;

        [[nodiscard]] const QJsonObject &getResult() const
        {
            return mResult;
        }

    private:
        QJsonObject mResult;
        QJsonObject dumpLovCommons(LovBase *pLov);

};
}
#endif //OST_LOVJSONDUMPER_H
