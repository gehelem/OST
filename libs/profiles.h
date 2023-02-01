#ifndef PROFILES_h_
#define PROFILES_h_
#include <QVariant>

class Profiles
{
    public:
        Profiles();
        ~Profiles();
    protected:
        QVariantMap getProfile(QString name)
        {
            if (mParams.contains(name)) return mParams[name].toMap();
            else return QVariantMap();
        }
        QString getType()
        {
            return mParams["type"].toString();
        }
    private:
        QVariantMap mParams;
}
;
#endif

