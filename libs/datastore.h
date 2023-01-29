#ifndef DATASTORE_h_
#define DATASTORE_h_
#include <QObject>
#include <QVariant>

class Datastore : public QObject
{
        Q_OBJECT

    public:
        Datastore();
        ~Datastore();
    protected:
        QVariant getPPP(QString name)
        {
            if (mProperties.contains(name)) return mProperties[name];
            else return QVariant();
        }
    private:
        QVariantMap mProperties;
        QVariantMap mParams;
}
;
#endif

