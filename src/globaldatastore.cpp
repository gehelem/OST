#include "globaldatastore.h"

GlobalDatastore::GlobalDatastore(const QString &name, const QString &label,
                                 const QString &profile, const QVariantMap &params)
    : Basemodule(name, label, profile, params)
{
    Q_INIT_RESOURCE(globaldatastore);
    loadOstPropertiesFromFile(":globaldatastore.json");
}

void GlobalDatastore::onAfterInit()
{
}

void GlobalDatastore::registerLov(const QString &propName, const QString &keyCol,
                                  const QString &labelCol, OST::LovBase* lov)
{
    mManagedLovs[propName] = { keyCol, labelCol, lov };
    rebuildLov(propName);
}

void GlobalDatastore::rebuildLov(const QString &propName)
{
    if (!mManagedLovs.contains(propName)) return;
    auto &ml = mManagedLovs[propName];
    OST::LovString* ls = static_cast<OST::LovString*>(ml.lov);
    ls->lovClear();
    int size = getGridSize(propName);
    for (int i = 0; i < size; i++)
    {
        QString key   = getGridString(propName, ml.keyCol,   i);
        QString label = getGridString(propName, ml.labelCol, i);
        if (!key.isEmpty()) ls->lovAdd(key, label);
    }
}

void GlobalDatastore::onExternalEvent(OST::ExtEvent event)
{
    // After any grid modification, keep managed LOVs in sync
    if (event.ev == OST::ExtEvType::GC || event.ev == OST::ExtEvType::GU ||
            event.ev == OST::ExtEvType::GD || event.ev == OST::ExtEvType::GH ||
            event.ev == OST::ExtEvType::GB || event.ev == OST::ExtEvType::GR)
    {
        if (mManagedLovs.contains(event.prpkey))
            rebuildLov(event.prpkey);
    }
}
