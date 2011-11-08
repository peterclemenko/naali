/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#include <QtGui>

#include "MenuDataModel.h"
#include "MenuDataItem.h"

MenuDataModel::MenuDataModel()
{
    //sets unique MenuDataItemId to nonzero value
    id_=1;
}

MenuDataModel::~MenuDataModel()
{
    qDeleteAll(menudataitems_);
//    foreach(MenuDataItem *menudataitem, menudataitems_)
//    {
//        delete menudataitem;
//    }
}

bool MenuDataModel::AddItem(MenuDataItem *item)
{
    menudataitems_.append(item);
    return true;
}
bool MenuDataModel::AddItem(QString mesh, QStringList materials)
{
    //qDebug() << "AddItem debug";
    MenuDataItem *item = new MenuDataItem(/*IdGenerator()*/);
    item->SetMaterialRef(materials);
    item->SetMeshRef(mesh);

    menudataitems_.append(item);
    return true;

}

bool MenuDataModel::AddItemToIndex(MenuDataItem *menudataitem, int index)
{
    if(index>0 && index < menudataitems_.count())
    {
        menudataitems_.insert(index-1, menudataitem);
        return true;
    }
    else if(index==0)
    {
        menudataitems_.insert(index, menudataitem);
        return true;
    }
    else
        return false;
}
bool MenuDataModel::AddItemToIndex(QString mesh, QStringList materials, int index)
{
    if(index>0 && index < menudataitems_.count())
    {
        MenuDataItem *item = new MenuDataItem();
        item->SetMaterialRef(materials);
        item->SetMeshRef(mesh);
        menudataitems_.insert(index-1, item);
        return true;
    }
    else
        return false;
}

MenuDataItem* MenuDataModel::GetMenuDataItem(int index)
{
    if(index < menudataitems_.count() && index >= 0)
        return menudataitems_.at(index);
    else
        return 0;
}

QObject* MenuDataModel::GetMenuDataItemRaw(int index)
{
    return dynamic_cast<QObject*>(GetMenuDataItem(index));
}

int MenuDataModel::GetNumberOfDataItems()
{
    return menudataitems_.count();
}

uint MenuDataModel::IdGenerator()
{
    return ++id_;
}
