#include "valueupdate.h"
#include <sstream>
namespace  OST
{
void ValueUpdate::visit(ValueInt *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toLongLong(), true);
}
void ValueUpdate::visit(ValueFloat *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toDouble(), true);
}
void ValueUpdate::visit(ValueBool *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toBool(), true);
}
void ValueUpdate::visit(ValueString *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate"))
    {
        qDebug() << "autoupdate" << pValue->label() << "/" << data;
        pValue->setAutoUpdate(data["autoupdate"].toBool());
    }
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    pValue->setValue(data["value"].toString(), true);
}
void ValueUpdate::visit(ValueLight *pValue, QVariantMap &data )
{
    if (data.contains("autoupdate")) pValue->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pValue->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pValue->setValue(Idle, true);
    if (data["value"].toInt() == 1 ) pValue->setValue(Ok, true);
    if (data["value"].toInt() == 2 ) pValue->setValue(Busy, true);
    if (data["value"].toInt() == 3 ) pValue->setValue(Error, true);
}
void ValueUpdate::visit(ValueImg *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueVideo *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueMessage *pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueGraph* pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValuePrg* pValue, QVariantMap &data )
{
    Q_UNUSED(pValue);
    Q_UNUSED(data);
}

void ValueUpdate::visit(ValueInt* pValue, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(data["val"].toInt());
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(data["val"].toInt(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(qlonglong(pValue->getGrid()[i]));
    }
}
void ValueUpdate::visit(ValueFloat* pValue, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(data["val"].toDouble());
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(data["val"].toDouble(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(pValue->getGrid()[i]);
    }

}
void ValueUpdate::visit(ValueBool* pValue, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(data["val"].toBool());
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(data["val"].toBool(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(pValue->getGrid()[i]);
    }
}
void ValueUpdate::visit(ValueString* pValue, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(data["val"].toString());
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(data["val"].toString(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(pValue->getGrid()[i]);
    }
}
void ValueUpdate::visit(ValueLight* pValue, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(IntToState(data["val"].toInt()));
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(IntToState(data["val"].toInt()), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(pValue->getGrid()[i]);
    }
}
void ValueUpdate::visit(ValueImg* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ValueUpdate::visit(ValueVideo* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);
}
void ValueUpdate::visit(ValueMessage* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ValueUpdate::visit(ValueGraph* pValue, QString &action, QVariantMap &data)
{
    Q_UNUSED(pValue);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ValueUpdate::visit(ValuePrg* pValue, QString &action, QVariantMap &data)
{
    qDebug() << "ValueUpdate::visit" << data;

    if (data.contains("arrayLimit"))
    {
        pValue->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pValue->gridAdd(pValue->value());
    }
    if (action == "newline")
    {
        pValue->gridAdd(IntToState(data["val"].toInt()));
    }
    if (action == "deleteline")
    {
        pValue->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pValue->gridUpdate(IntToState(data["val"].toInt()), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pValue->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pValue->getGrid().size(); ++i) mGrid.append(pValue->getGrid()[i]);
    }

}

void GridUpdate::visit(GridInt* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridBool* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridFloat* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridString* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridLight* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridImg* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridVideo* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridMessage* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridGraph* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}
void GridUpdate::visit(GridPrg* pGrid, QString &action, int &line)
{
    if(action == "add")
    {
        pGrid->items.append(pGrid->mElement->value());
        return;
    }
    if(line > pGrid->items.size() - 1)
    {
        qDebug() << "GridUpdate - index greater than size (" << line << ":" << pGrid->items.size() - 1 << ") - " << action << " - "
                 << pGrid->mElement->label();
        return;
    }
    if(action == "del")
    {
        pGrid->items.removeAt(line);
        return;
    }
    if(action == "upd")
    {
        pGrid->items[line] = pGrid->mElement->value();
        return;
    }
    if(action == "fet")
    {
        pGrid->mElement->setValue(pGrid->items[line], false);
        return;
    }
    qDebug() << "GridUpdate - unhandled action " << action << " - " << pGrid->mElement->label();
}



}
