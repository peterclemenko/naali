/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#pragma once

#include "MenuRendererInterface.h"
#include "SceneFwd.h"

class Entity;
class RaycastResult;
class EC_MenuContainer;
class EC_MenuItem;
class EC_Mesh;
class float3;

class RingMenuRenderer: public MenuRendererInterface
{
    Q_OBJECT

public:
    RingMenuRenderer(EC_MenuContainer *menucontainer, MenuDataModel *datamodel);
    ~RingMenuRenderer();

    void HandleMouseInput(MouseEvent *mouse, RaycastResult* result);

public slots:
    void KineticScroller();

private:
    QList<EC_MenuItem*> MenuItemList_;
//    EC_Mesh *ringmesh_;
    EC_MenuContainer *menuContainer_;

    QTimer *scrollerTimer_;
    int scrollerTimer_Interval;

    bool menuClicked_;
    //bool subMenu_clicked_;
    bool subMenu_;
    //bool subMenuIsScrolling;
    //bool startingPositionSaved_;
//    bool menuIsRotating_;

    float speed_;
    //float radius_;
    float item_offset_;
//    float subMenuRadius_;
    float rotationDirection_;
//    InputContextPtr input_;
    int selected_;
    int previousSelected_;
//    int subMenuItemSelected_;
    int menulayer_;
    int itemToRotate_;

    float3 CalculateItemPosition(float phi, bool isSelected=false);
};
