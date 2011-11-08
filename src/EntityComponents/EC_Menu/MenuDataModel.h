/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */
#pragma once
//#ifndef incl_EC_Menu_MenuDataModel_h
//#define incl_EC_Menu_MenuDataModel_h

#include <QString>
#include <QList>
#include <QStringList>

class MenuDataItem;

class MenuDataModel : public QObject
{
    Q_OBJECT
public:
    MenuDataModel();
    ~MenuDataModel();

public slots:
    bool AddItem(MenuDataItem*);
    bool AddItem(QString mesh, QStringList materials=QStringList());

    bool AddItemToIndex(MenuDataItem *item, int index=0);
    bool AddItemToIndex(QString mesh, QStringList materials, int index=0);

    MenuDataItem* GetMenuDataItem(int index);
    QObject* GetMenuDataItemRaw(int index);

    int GetNumberOfDataItems();

private:
    QList<MenuDataItem*> menudataitems_;
    uint IdGenerator();

    uint id_;

};

//#endif
