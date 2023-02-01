#ifndef PROFILES_h_
#define PROFILES_h_
#include <QObject>
#include <QVariant>

class Baseroot
{
    public:
        Baseroot();
        ~Baseroot();
    protected:
        void sendMessage(const QString &pMessage);
        virtual void OnModuleEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                                   const QVariantMap &eventData) {};
    private:
}
;
#endif

