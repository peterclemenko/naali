
/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */
#pragma once

//#ifndef incl_EC_Menu_MenuDataItem_h
//#define incl_EC_Menu_MenuDataItem_h

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>
#include "IComponent.h"
//#include <IComponent.h>

class MenuDataItem : public QObject
{
    Q_OBJECT
public:
    MenuDataItem(MenuDataItem *parent = 0);
    ~MenuDataItem();

//    //! Gravity toggle. If true(default), gravity has effect on this body.
//    Q_PROPERTY(bool gravityEnabled READ getgravityEnabled WRITE setgravityEnabled)
//    DEFINE_QPROPERTY_ATTRIBUTE(bool, gravityEnabled)

public slots:
    bool AddChildren(QString meshref, QStringList materialref=QStringList());
    //bool AddChildren(QWidget *widget, QString meshref=QString(), QStringList materialref=QStringList());
    bool AddChildren(QWidget *widget, int widgetsubmesh, QString meshref=QString(), QStringList materialref=QStringList());
    MenuDataItem* Parent();

    bool SetMeshRef(QString meshref);
    bool SetMaterialRef(QStringList materialref);
    bool SetWidget(QWidget *widget, int widgetsubmesh);

    int GetChildCount();
    MenuDataItem* GetChildDataItem(int index);
    QObject* GetChildDataItemRaw(int index);
    MenuDataItem* GetParentDataItem();

    QString GetMeshRef();
    QStringList GetMaterialRef();
    QWidget* GetWidget(){return widget_;}

private:
    MenuDataItem *parent_;
    QList<MenuDataItem*> childItems_;
    QString meshreference_;
    QStringList materialreference_;
    QWidget *widget_;
    int widgetsubmesh_;
    uint item_id;

signals:
    void DataChanged();

};
//#endif
