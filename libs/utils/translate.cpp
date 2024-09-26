#include "translate.h"

namespace  OST
{
Translate* translate_ = nullptr;

Translate::Translate(void)
{
    QFile jsonFile("translations.json");
    if (jsonFile.open(QFile::ReadOnly))
    {
        qDebug() << "***************************************************** Loading translation file";
        QJsonDocument d = QJsonDocument().fromJson(jsonFile.readAll());
        jsonFile.close();
        mTranslations = d.toVariant().toMap();
        qDebug() << mTranslations;
    };
};

QString Translate::translate(QString val)
{
    if (!mTranslations.contains(val))
    {
        qDebug() << "new text to translate : " << val;
        QVariantMap t;
        t["fr"] = "à traduire";
        t["en"] = "to be translated";
        mTranslations[val] = t;

        QFile jsonFile("translations_pending.json");
        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(QJsonDocument::fromVariant(mTranslations).toJson());
        jsonFile.close();
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





}
