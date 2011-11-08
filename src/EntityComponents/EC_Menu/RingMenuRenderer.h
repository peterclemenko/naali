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
    //Q_OBJECT

public:
    RingMenuRenderer(EC_MenuContainer *menucontainer, MenuDataModel *datamodel);
    ~RingMenuRenderer();

    void HandleMouseInput(MouseEvent *);
//    void EntityClicked(Entity*, Qt::MouseButton, RaycastResult*);
//    void EntityMouseMove(Entity*, Qt::MouseButton, RaycastResult*);
//    void EntityClickReleased(Entity*, Qt::MouseButton, RaycastResult*);

    void Initialize();

protected:
    void SetMenuPosition();

private:
    QList<EC_MenuItem*> MenuItemList_;
    EC_Mesh *ringmesh_;
    EC_MenuContainer *menuContainer_;

    float3 CalculateItemPosition(float phi, bool isSelected=false);
};
