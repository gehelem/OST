#include "translate.h"

namespace  OST
{
Translate* translate_ = nullptr;

Translate::Translate(void)
{
    QFile jsonFile(":translations.json");
    if (jsonFile.open(QFile::ReadOnly))
    {
        QJsonDocument d = QJsonDocument().fromJson(jsonFile.readAll());
        jsonFile.close();
        mTranslations = d.toVariant().toMap();
    };
};

QString Translate::translate(QString val)
{
    if (!mTranslations.contains(val))
    {
        if (!mPendingTranslations.contains(val))
        {
            qDebug() << "new text to translate : " << val;
            QVariantMap t;
            t["fr"] = "à traduire";
            t["en"] = val;
            mPendingTranslations[val] = t;

            QFile jsonFile("translations_pending.json");
            jsonFile.open(QFile::WriteOnly);
            jsonFile.write(QJsonDocument::fromVariant(mPendingTranslations).toJson());
            jsonFile.close();
        }
        return "[" + val + "]";
    }
    else
    {
        return mTranslations[val].toMap()[mLng].toString();
    }


};


Translate *Translate::GetInstance(void)
{
    if(translate_ == nullptr)
    {
        translate_ = new Translate();
    }
    return translate_;
}

void Translate:: setLanguage(QString lng)
{
    mLng = lng;
}




}
