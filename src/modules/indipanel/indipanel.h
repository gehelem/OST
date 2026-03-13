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

    public slots:
        //void OnSetPropertyText(TextProperty* prop) override;

    private:
        void newDevice      (INDI::BaseDevice dp) override;
        void removeDevice   (INDI::BaseDevice dp) override;
        void newProperty    (INDI::Property property) override;
        void removeProperty (INDI::Property property) override;
        void updateProperty (INDI::Property property) override;
        void newMessage     (INDI::BaseDevice dp, int messageID) override;
        void onExternalEvent(OST::ExtEvent event) override;
        QPointer<fileio> _image;
        FITSImage::Statistic stats;

};

extern "C" MODULE_INIT IndiPanel *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif
