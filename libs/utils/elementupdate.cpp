#include "elementupdate.h"
#include <sstream>
namespace  OST
{
void ElementUpdate::visit(ElementInt *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toLongLong(), true);
}
void ElementUpdate::visit(ElementFloat *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toDouble(), true);
}
void ElementUpdate::visit(ElementBool *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toBool(), true);
}
void ElementUpdate::visit(ElementString *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate"))
    {
        qDebug() << "autoupdate" << pElement->label() << "/" << data;
        pElement->setAutoUpdate(data["autoupdate"].toBool());
    }
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    pElement->setValue(data["value"].toString(), true);
}
void ElementUpdate::visit(ElementLight *pElement, QVariantMap &data )
{
    if (data.contains("autoupdate")) pElement->setAutoUpdate(data["autoupdate"].toBool());
    if (!data.contains("value"))
    {
        qDebug() << "no value for " << pElement->label();
        return;
    }
    if (data["value"].toInt() == 0 ) pElement->setValue(Idle, true);
    if (data["value"].toInt() == 1 ) pElement->setValue(Ok, true);
    if (data["value"].toInt() == 2 ) pElement->setValue(Busy, true);
    if (data["value"].toInt() == 3 ) pElement->setValue(Error, true);
}
void ElementUpdate::visit(ElementImg *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementVideo *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementMessage *pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementGraph* pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementPrg* pElement, QVariantMap &data )
{
    Q_UNUSED(pElement);
    Q_UNUSED(data);
}

void ElementUpdate::visit(ElementInt* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(data["val"].toInt());
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(data["val"].toInt(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(qlonglong(pElement->getGrid()[i]));
    }
}
void ElementUpdate::visit(ElementFloat* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(data["val"].toDouble());
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(data["val"].toDouble(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(pElement->getGrid()[i]);
    }

}
void ElementUpdate::visit(ElementBool* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(data["val"].toBool());
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(data["val"].toBool(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(pElement->getGrid()[i]);
    }
}
void ElementUpdate::visit(ElementString* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(data["val"].toString());
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(data["val"].toString(), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(pElement->getGrid()[i]);
    }
}
void ElementUpdate::visit(ElementLight* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(IntToState(data["val"].toInt()));
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(IntToState(data["val"].toInt()), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(pElement->getGrid()[i]);
    }
}
void ElementUpdate::visit(ElementImg* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ElementUpdate::visit(ElementVideo* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);
}
void ElementUpdate::visit(ElementMessage* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ElementUpdate::visit(ElementGraph* pElement, QString &action, QVariantMap &data)
{
    Q_UNUSED(pElement);
    Q_UNUSED(action);
    Q_UNUSED(data);

}
void ElementUpdate::visit(ElementPrg* pElement, QString &action, QVariantMap &data)
{
    if (data.contains("arrayLimit"))
    {
        pElement->setArrayLimit(data["arrayLimit"].toInt());
    }
    if (action == "push")
    {
        pElement->gridAdd(pElement->value());
    }
    if (action == "newline")
    {
        pElement->gridAdd(IntToState(data["val"].toInt()));
    }
    if (action == "deleteline")
    {
        pElement->gridDel(data["i"].toInt());
    }
    if (action == "updateline")
    {
        pElement->gridUpdate(IntToState(data["val"].toInt()), data["i"].toInt(), true);
    }
    if (action == "cleargrid")
    {
        pElement->gridClear();
    }
    if (action == "getgrid")
    {
        mGrid.clear();
        for (int i = 0; i < pElement->getGrid().size(); ++i) mGrid.append(pElement->getGrid()[i]);
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


QJsonObject GridJsonDumper::dumpGridCommons(GridBase *pGrid)
{
    QJsonObject json;
    json["type"] = pGrid->getType();
    return json;

}
void GridJsonDumper::visit(GridInt* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    if (pGrid->items.size() > 0)
    {
        QJsonArray arr;
        foreach (int val, pGrid->items)
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;
}
void GridJsonDumper::visit(GridString* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    if (pGrid->items.size() > 0)
    {
        QJsonArray arr;
        foreach (QString val, pGrid->items)
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;

}
void GridJsonDumper::visit(GridBool* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    if (pGrid->items.size() > 0)
    {
        QJsonArray arr;
        foreach (bool val, pGrid->items)
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;

}
void GridJsonDumper::visit(GridImg* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}
void GridJsonDumper::visit(GridFloat* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    if (pGrid->items.size() > 0)
    {
        QJsonArray arr;
        foreach (double val, pGrid->items)
        {
            QVariant v = QVariant::fromValue(val);
            arr.append(v.toJsonValue());
        }

        json["gridvalues"] = arr;
    }
    mResult = json;
}
void GridJsonDumper::visit(GridMessage* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}
void GridJsonDumper::visit(GridVideo* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}
void GridJsonDumper::visit(GridLight* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}
void GridJsonDumper::visit(GridPrg* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}
void GridJsonDumper::visit(GridGraph* pGrid, QString &action, int &line)
{
    QJsonObject json = dumpGridCommons(pGrid);
    mResult = json;
}


}
