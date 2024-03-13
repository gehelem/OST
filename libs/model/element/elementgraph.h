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

class ValueGraph: public ValueTemplate<GraphDefs>
{
        Q_OBJECT

    public:

        void accept(ValueVisitor* pVisitor)  override
        {
            pVisitor->visit(this);
        }
        ValueGraph(ElementBase* element): ValueTemplate<GraphDefs>(element) {}
        ~ValueGraph() {}
        void updateValue() override
        {
            value = static_cast<ElementGraph*>(pElement)->value();
        }
        void updateElement(const bool &emitEvent) override
        {
            static_cast<ElementGraph*>(pElement)->setValue(value, emitEvent);
        }

};
}
#endif
