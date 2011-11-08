/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#include "MenuDataItem.h"

MenuDataItem::MenuDataItem(MenuDataItem *parent)
{
    //item_id = itemId;
    parent_ = parent;
    widget_=0;

}

MenuDataItem::~MenuDataItem()
{
    qDeleteAll(childItems_);
}

bool MenuDataItem::AddChildren(QString meshref, QStringList materialref)
{
    MenuDataItem *childitem = new MenuDataItem(this);
    childitem->SetMeshRef(meshref);
    childitem->SetMaterialRef(materialref);
    childItems_.append(childitem);

    return true;
}
bool MenuDataItem::AddChildren(QWidget *widget,int widgetsubmesh, QString meshref, QStringList materialref)
{
    MenuDataItem *childitem = new MenuDataItem(this);
//    if(meshref.isEmpty())
//        meshref.append("rect_plane.mesh");
    childitem->SetMeshRef(meshref);
    childitem->SetMaterialRef(materialref);
    childitem->SetWidget(widget, widgetsubmesh);
    childItems_.append(childitem);

    return true;
}

int MenuDataItem::GetChildCount()
{
    return childItems_.count();
}

MenuDataItem* MenuDataItem::GetChildDataItem(int index)
{
    if (index < childItems_.count() && index >= 0)
        return childItems_.at(index);
    else
        return 0;
}

QObject* MenuDataItem::GetChildDataItemRaw(int index)
{
    if (index < childItems_.count() && index >= 0)
        return childItems_.at(index);
    else
        return 0;
}

MenuDataItem* MenuDataItem::GetParentDataItem()
{
    if (parent_)
        return parent_;
    else
        return 0;
}

QStringList MenuDataItem::GetMaterialRef()
{
    return materialreference_;
}

QString MenuDataItem::GetMeshRef()
{
    return meshreference_;
}

MenuDataItem* MenuDataItem::Parent()
{
    return parent_;
}

bool MenuDataItem::SetMeshRef(QString meshref)
{
    meshreference_ = meshref;
    emit DataChanged();
    return true;
}

bool MenuDataItem::SetMaterialRef(QStringList materialref)
{
    materialreference_ = materialref;
    emit DataChanged();
    return true;
}

bool MenuDataItem::SetWidget(QWidget *widget, int widgetsubmesh)
{
    widget_ = widget;
    widgetsubmesh_ = widgetsubmesh;
    emit DataChanged();
    return true;
}
