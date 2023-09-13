#ifndef LOVBASE_h_
#define LOVBASE_h_

#include <lovvisitor.h>
#include <common.h>

namespace  OST
{

class LovVisitor;

class LovBase: public QObject
{

        Q_OBJECT

    public:
        virtual void accept(LovVisitor* pVisitor) = 0;

        LovBase(const QString &label)
        {
            mLabel = label;
        }
        ~LovBase() {}

        virtual QString getType() = 0;
        QString label()
        {
            return mLabel;
        }
        QString getKey()
        {
            return mKey;
        }
        void setKey(QString key)
        {
            mKey = key;
        }
    private:
        QString mLabel = "change me";
        QString mKey = "";
    signals:
        void lovChanged(OST::LovBase*);
        void sendMessage(OST::MsgLevel, QString);



};

}

#endif

