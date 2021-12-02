#ifndef PROPERTIES_H
#define PROPERTIES_H
#include <QtCore>


class Devcat : public QObject {
Q_OBJECT
public:
    Devcat( QString name, QString label,QObject* parent = nullptr)
        : QObject(parent), _name(name), _label(label){setObjectName(name);}
    virtual ~Devcat() = default;

    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
private:
    QString _name;
    QString _label;
};


class Property : public QObject {
Q_OBJECT
public:
    Property( QString name, QString label, int permission, int state,QObject* parent = nullptr);
    virtual ~Property() = default;

    [[nodiscard]] inline const QString &getName() const { return _name; }
    [[nodiscard]] inline const QString &getLabel() const { return _label; }
    [[nodiscard]] inline int getPermission() const { return _permission; }
    [[nodiscard]] inline int getState() const { return _state; }
    void emitProp(void);

private:
    QString _name;
    QString _label;
    int _permission;
    int _state;
signals:
    void propertyCreated(Property *prop)    ;
    void propertyModified(Property *prop)    ;
    void propertyDeleted(Property *prop)    ;

};

#endif // PROPERTIES_H
