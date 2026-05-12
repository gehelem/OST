#ifndef GLOBALDATASTORE_H
#define GLOBALDATASTORE_H

#include <basemodule.h>
#include <lovbase.h>
#include <lovstring.h>

/*!
 * Shared read/write datastore owned by the Controller.
 * Holds equipment data (optics, sensors, …) accessible by all modules.
 *
 * Modules receive change notifications via the existing otherModuleEvent
 * mechanism (mod == getModuleName() of this instance).
 *
 * Modules read data via the public Basemodule grid utilities:
 *   mGlobalDatastore->findGridRow(…)
 *   mGlobalDatastore->getGridString(…)
 *   etc.
 */
class GlobalDatastore : public Basemodule
{
    Q_OBJECT

public:
    GlobalDatastore(const QString& name, const QString& label,
                    const QString& profile, const QVariantMap& params);

    /*!
     * Associate a controller LOV with a grid column so it is rebuilt
     * automatically whenever the grid changes.
     *
     * @param propName   Property key (e.g. "optics")
     * @param keyCol     Element key used as LOV key   (e.g. "name")
     * @param labelCol   Element key used as LOV label (e.g. "name")
     * @param lov        Pointer to the controller LovString to maintain
     */
    void registerLov(const QString& propName, const QString& keyCol,
                     const QString& labelCol, OST::LovBase* lov);

    void onAfterInit() override;

protected:
    void onExternalEvent(OST::ExtEvent event) override;

private:
    struct ManagedLov
    {
        QString     keyCol;
        QString     labelCol;
        OST::LovBase* lov;
    };

    QMap<QString, ManagedLov> mManagedLovs;

    void rebuildLov(const QString& propName);
};

#endif
