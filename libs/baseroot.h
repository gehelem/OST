#ifndef BASEROOT_h_
#define BASEROOT_h_
#include <QObject>
#include <QVariant>

class Baseroot
{
    public:
        Baseroot();
        ~Baseroot();
    protected:
        void sendMessage(const QString &pMessage);
        virtual void OnModuleEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                   const QVariantMap &pEventData)
        {
            Q_UNUSED(pEventType);
            Q_UNUSED(pEventModule);
            Q_UNUSED(pEventKey);
            Q_UNUSED(pEventData);
        };
    private:
}
;
#endif

