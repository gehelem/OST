#ifndef PROPERTYMULTI_h_
#define PROPERTYMULTI_h_

#include <propertybase.h>
#include <elementfactory.h>

namespace  OST
{

typedef enum
{
    OneOfMany = 0,
    AtMostOne,
    Any
} SwitchsRule;
inline SwitchsRule IntToRule(int val )
{
    if (val == 0) return OneOfMany;
    if (val == 1) return AtMostOne;
    if (val == 2) return Any;
    qDebug() << "Cant convert " << val << " to OST::SwitchsRule (0-2) - defaults to any";
    return Any;
}


class PropertyMulti: public PropertyBase
{

        Q_OBJECT

    public:
        void accept(PropertyVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(PropertyVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }


        PropertyMulti(const QString &key, const QString &label, const Permission &permission, const QString &level1,
                      const QString &level2,
                      const QString &order, const bool &hasProfile, const bool &hasGrid
                     );
        ~PropertyMulti();
        [[nodiscard]] SwitchsRule rule() const
        {
            return mRule;
        }
        void setRule(SwitchsRule rule)
        {
            mRule = rule;
        }

        QMap<QString, ElementBase*>* getElts()
        {
            return &mElts;
        }
        ElementBase* getElt(QString pElement)
        {
            if (!mElts.contains(pElement))
            {
                qDebug() << label() << " - getElt - element " << pElement << " does not exists.";
                return nullptr;
            }
            return mElts[pElement];
        }
        bool setElt(QString key, QVariant val);
        void addElt(QString key, ElementBase* pElt)
        {
            if (mElts.contains(key))
            {
                qDebug() << label() << " - addElt - element " << key << " already exists";
                return;
            }
            mElts[key] = pElt;
            mGridHeaders.append(key);
            connect(mElts[key], &ElementBase::eltChanged, this, &PropertyMulti::OnEltChanged);
            connect(mElts[key], &ElementBase::valueSet, this, &PropertyMulti::OnValueSet);
            connect(mElts[key], &ElementBase::listChanged, this, &PropertyMulti::OnListChanged);
            connect(mElts[key], &ElementBase::lovChanged, this, &PropertyMulti::OnLovChanged);
            connect(mElts[key], &ElementBase::sendMessage, this, &PropertyMulti::OnMessage);
        }
        void deleteElt(QString key)
        {
            if (!mElts.contains(key))
            {
                qDebug() << label() << " - deleteElt - element " << key << " doesn't exist";
                return;
            }
            mElts.remove(key);
            emit propertyEvent("ap", key, this);

        }
        void push();
        void newLine(const QVariantMap &pValues);
        void deleteLine(const int i);
        void updateLine(const int i, const QVariantMap &pValues);
        void clearGrid();
        QList<QString> getGridHeaders()
        {
            return mGridHeaders;
        }
        QList<QList<ValueBase*>> getGrid()
        {
            return mGrid;
        }
    public slots:
        void OnValueSet(ElementBase*)
        {
            emit valueSet(this);
        }
        void OnEltChanged(ElementBase*)
        {
            emit eltChanged(this);
        }
        void OnListChanged(ElementBase*)
        {
            emit propertyEvent("ap", key(), this);
        }
        void OnLovChanged(ElementBase*)
        {
            emit propertyEvent("ap", key(), this);
        }
        void OnMessage(MsgLevel l, QString m)
        {
            switch (l)
            {
                case Info:
                    sendInfo(this->key() + "-" + m);
                    break;
                case Warn:
                    sendWarning(this->key() + "-" + m);
                    break;
                case Err:
                    sendError(this->key() + "-" + m);
                    break;
                default:
                    sendError(this->key() + "-" + m);
                    break;
            }
        }
        bool getShowElts()
        {
            return mShowElts;
        }
        void setShowElts(bool b)
        {
            mShowElts = b;
        }
        bool hasGrid()
        {
            return mHasGrid;
        }
        void setHasGrid(bool b)
        {
            mHasGrid = b;
        }
        bool getShowGrid()
        {
            return mShowGrid;
        }
        void setShowGrid(bool b)
        {
            mShowGrid = b;
        }
        int getGridLimit()
        {
            return mGridLimit;
        }
        void setGridLimit(int limit)
        {
            if (limit > 0) mGridLimit = limit;
        }
        bool hasGraph()
        {
            return mHasGraph;
        }
        void setHasGraph(bool b)
        {
            mHasGraph = b;
        }
        GraphDefs getGraphDefs(void)
        {
            return mGraphDefs;
        }
        void setGraphDefs(GraphDefs defs);
    private:
        bool mHasGrid = false;
        bool mShowGrid = false;
        bool mShowElts = true;
        int mGridLimit = 0;
        bool mHasGraph = false;
        GraphDefs mGraphDefs;
        SwitchsRule mRule = SwitchsRule::Any;
        QMap<QString, ElementBase*> mElts;
        QList<QString> mGridHeaders;
        QList<QList<ValueBase*>> mGrid;


};

}
#endif

