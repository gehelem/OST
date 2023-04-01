#ifndef BASICPROPERTY_h_
#define BASICPROPERTY_h_

#include <QObject>
#include <QList>
#include <QVariant>
#include <QtCore>


class BasicProperty : public QObject
{
        Q_OBJECT
        Q_PROPERTY(Priority priority READ priority WRITE setPriority NOTIFY priorityChanged)
        Q_PROPERTY(QString label MEMBER mLabel)
        //Q_PROPERTY(QVariant value MEMBER mValue NOTIFY valueChanged)

    public:
        BasicProperty();
        ~BasicProperty();

        enum Priority { High, Low, VeryHigh, VeryLow };
        Q_ENUM(Priority)

        void setPriority(Priority priority)
        {
            m_priority = priority;
            emit priorityChanged( priority);
        }
        Priority priority() const
        {
            return m_priority;
        }

    signals:
        //void propertyCreated(BasicProperty);
        void priorityChanged(BasicProperty::Priority);
        //void valueChanged(QVariant);

    private:
        QString mLabel;
        Priority m_priority;
        QVariant mValue;

};

#endif

