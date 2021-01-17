#ifndef PROPERTIES_H
#define PROPERTIES_H
#include <QtCore>

class OstProperties : public QObject
{
    Q_OBJECT
    public:
        OstProperties(void);
        ~OstProperties();

        QString  getText(const char *name);
        bool     setText(const char *name,QString text);
        double   getNumber(const char *name);
        bool     setNumber(const char *name,double num);
        bool     AssignCategory(const char *name,const char *category);
        bool     AssignGroup(const char *name,const char *group);
};


#endif // PROPERTIES_H
