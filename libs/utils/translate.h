#ifndef TRANSLATE_h_
#define TRANSLATE_h_

#include <QtCore>
namespace  OST
{

class Translate
{
    protected:
        Translate(void);
    public:
        Translate(Translate &other) = delete;
        void setLanguage(QString lng);
        void operator=(const Translate &) = delete;
        static Translate *GetInstance(void);
        QString translate(QString val);
    private:
        QVariantMap mTranslations;
        QVariantMap mPendingTranslations;
        QString mLng = "fr";

};

}
#endif
