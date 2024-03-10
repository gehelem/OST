#include "elementgraph.h"
namespace  OST
{
ElementGraph::ElementGraph(const QString &label, const QString &order, const QString &hint)
    : ElementSingleNotNumeric(label, order, hint)
{
}
ElementGraph::~ElementGraph()
{
}
void ElementGraph::setGraphDefs(GraphDefs defs)
{

    switch (defs.type)
    {
        case none:
            mGraphDefs.params = QVariantMap();
            mGraphDefs.type = none;
            return;
            break;
        case XY:
            if (!defs.params.contains("X"))
            {
                qDebug() << "XY graph definition error : param should contain X binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "XY graph definition error : param should contain Y binding";
                return;
            }
            break;
        case DY:
            if (!defs.params.contains("D"))
            {
                qDebug() << "DY graph definition error : param should contain D binding";
                return;
            }
            if (!defs.params.contains("Y"))
            {
                qDebug() << "DY graph definition error : param should contain Y binding";
                return;
            }
            break;
        case PHD:
            if (!defs.params.contains("D"))
            {
                qDebug() << "PHD graph definition error : param should contain D binding";
                return;
            }
            if (!defs.params.contains("RA"))
            {
                qDebug() << "PHD graph definition error : param should contain RA binding";
                return;
            }
            if (!defs.params.contains("DE"))
            {
                qDebug() << "PHD graph definition error : param should contain DE binding";
                return;
            }
            if (!defs.params.contains("pRA"))
            {
                qDebug() << "PHD graph definition error : param should contain pRA binding";
                return;
            }
            if (!defs.params.contains("pDE"))
            {
                qDebug() << "PHD graph definition error : param should contain pDE binding";
                return;
            }
            break;
        default:
            qDebug() << "setGraphDefs unknown graph def";
            mGraphDefs.params = QVariantMap();
            mGraphDefs.type = none;
            return;
            break;
    }
    mGraphDefs = defs;
}


}
