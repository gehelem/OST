#ifndef ELEMENTGRAPH_h
#define ELEMENTGRAPH_h

#include <elementsingle.h>

namespace  OST
{

class ElementGraph: public ElementSingleNotNumeric<GraphDefs>
{

        Q_OBJECT

    public:
        void accept(ElementVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override
        {
            pVisitor->visit(this, action, data);
        }

        ElementGraph(const QString &label, const QString &order, const QString &hint);
        ~ElementGraph();
        QString getType() override
        {
            return "graph";
        }

        GraphDefs getGraphDefs(void)
        {
            return mGraphDefs;
        }
        void setGraphDefs(GraphDefs defs);
        void setGraphParams(QVariantMap params);

    private:
        GraphDefs mGraphDefs;

};

class GridGraph: public GridTemplate<GraphDefs>
{

        Q_OBJECT

    public:

        void accept(GridVisitor* pVisitor, QString &action, int &line)  override
        {
            pVisitor->visit(this, action, line);
        }

        GridGraph(ElementBase* value): GridTemplate<GraphDefs>(value), mElement(static_cast<ElementGraph*>(value)) {}
        ~GridGraph() {}

        QString getType() override
        {
            return "graph";
        }
        ElementGraph* mElement;
};


}
#endif
