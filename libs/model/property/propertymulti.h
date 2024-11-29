#ifndef PROPERTYMULTI_h_
#define PROPERTYMULTI_h_

#include <propertybase.h>
#include <elementfactory.h>

namespace  OST
{

class PropertyMulti: public PropertyBase
{

        Q_OBJECT

    public:
        void accept(PropertyVisitor *pVisitor) override;
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override;

        PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                      const QString &level2,
                      const QString &order, const bool &hasProfile, const bool &hasGrid
                     );
        ~PropertyMulti();

        [[nodiscard]] SwitchsRule rule() const;
        void setRule(SwitchsRule rule);
        QMap<QString, ElementBase*>* getElts();
        ElementBase* getElt(QString pElement);
        bool setElt(QString key, QVariant val);
        void addElt(QString key, ElementBase* pElt);
        void deleteElt(QString key);
        void push();
        void newLine(const QVariantMap &pValues);
        bool updateLine(const int i, const QVariantMap &pValues);
        bool deleteLine(const int i);
        bool updateLine(const int i);
        void clearGrid();
        QList<QString> getGridHeaders();
        QList<QMap<QString, ValueBase*>> getGrid();
        bool getShowElts();
        void setShowElts(bool b);
        bool hasGrid();
        void setHasGrid(bool b);
        bool getShowGrid();
        void setShowGrid(bool b);
        int getGridLimit();
        void setGridLimit(int limit);
        bool hasGraph();
        void setHasGraph(bool b);
        GraphDefs getGraphDefs(void);
        void setGraphDefs(GraphDefs defs);
        bool swapLines(int l1, int l2);
        bool fetchLine(int l1);
        bool autoUpDown(void);
        void setAutoUpDown(bool b);
        bool autoSelect(void);
        void setAutoSelect(bool b);


    public slots:
        void OnValueSet(ElementBase*);
        void OnEltChanged(ElementBase*);
        void OnListChanged(ElementBase*);
        void OnLovChanged(ElementBase*);
        void OnMessage(MsgLevel l, QString m);
    private:
        bool mHasGrid = false;
        bool mShowGrid = false;
        bool mShowElts = true;
        int mGridLimit = 1000;
        bool mHasGraph = false;
        GraphDefs mGraphDefs;
        SwitchsRule mRule = SwitchsRule::Any;
        QMap<QString, ElementBase*> mElts;
        QList<QMap<QString, ValueBase*>> mGrid;
        bool mAutoUpDown = true;
        bool mAutoSelect = true;


};

}
#endif

