/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#pragma once

#include "MenuRendererInterface.h"
class Entity;
class RaycastResult;

class ShelveMenuRenderer: public MenuRendererInterface
{

public:
    ShelveMenuRenderer(MenuDataModel *datamodel);
    ~ShelveMenuRenderer();

    void HandleMouseInput(MouseEvent *, RaycastResult* result);

    void EntityClicked(Entity*, Qt::MouseButton, RaycastResult*);
    void EntityMouseMove(Entity*, Qt::MouseButton, RaycastResult*);
    void EntityClickReleased(Entity*, Qt::MouseButton, RaycastResult*);

protected:
    void SetMenuPosition();

};
