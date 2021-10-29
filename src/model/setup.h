#ifndef OST_SETUP_H
#define OST_SETUP_H

#include <QMap>
#include "devcat.h"

class Setup {

public:
    virtual ~Setup();
    void addDevcat(const Devcat *pDevcat);
    void removeDevcatByName(const QString &devcatName);
    const Devcat* getDevcatByName(const QString &devcatName);
    void cleanup();
private:
    QMap<QString, const Devcat*> _devcatsMap;

};


#endif //OST_SETUP_H
