#ifndef INDIPANEL_MODULE_h_
#define INDIPANEL_MODULE_h_
#include <indimodule.h>
#include <fileio.h>

#if defined(INDIPANEL_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT IndiPanel : public IndiModule
{
        Q_OBJECT

    public:
        IndiPanel(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~IndiPanel();
        void onNewDevice      (INDI::BaseDevice dp) override ;
        void onRemoveDevice   (INDI::BaseDevice dp) override;
        void onNewProperty    (INDI::Property property) override;
        void onRemoveProperty (INDI::Property property) override;
        void onUpdateProperty (INDI::Property property) override ;

    public slots:
        //void OnSetPropertyText(TextProperty* prop) override;

    private:
        void newMessage     (INDI::BaseDevice dp, int messageID) override;
        void onExternalEvent(OST::ExtEvent event) override;
        QString groupOrder(const QString &group);
        QPointer<fileio> _image;
        FITSImage::Statistic stats;
        // Per-property tiebreaker appended to groupOrder()'s own rank prefix,
        // so two properties/groups sharing a rank still sort in arrival
        // order instead of landing on an unstable tie (see onNewProperty()).
        int _orderSeq = 0;

};

extern "C" MODULE_INIT IndiPanel *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif
