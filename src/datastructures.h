#ifndef DATASTRUCTURES_h_
#define DATASTRUCTURES_h_
#include <QObject>
#include <QtCore>
#include <boost/log/trivial.hpp>
#include <indiapi.h>

class ModuleData : public QObject
{
    Q_OBJECT
public:
    ModuleData() {}
    ~ModuleData() = default;
    [[nodiscard]] inline const QString &getName()  const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
                  inline void setName (QString name )  { _name =name;  }
                  inline void setLabel(QString label)  { _label=label; }

private:
    QString _name;
    QString _label;
};

class PropertyData : public QObject
{
    Q_OBJECT

public:
    PropertyData() {}
    ~PropertyData() = default;
    [[nodiscard]] inline const QString &getName()  const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
    [[nodiscard]] inline const QString &getValue() const { return _value; }
                  inline void setName (QString name )  { _name =name;  }
                  inline void setLabel(QString label)  { _label=label; }
                  inline void setValue(QString value)  { _value=value; }
private:
    QString _name;
    QString _label;
    IPerm _permission = IP_RO;
    QString _value = "";
};


#endif
