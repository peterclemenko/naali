/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#pragma once

#include "InputAPI.h"
#include "MenuDataModel.h"
class Entity;
class RaycastResult;

class MenuRendererInterface
{

public:
    MenuRendererInterface(MenuDataModel *datamodel){menuDataModel_ = datamodel;}
    virtual ~MenuRendererInterface() = 0;

    virtual void HandleMouseInput(MouseEvent*) = 0;

//    virtual void EntityClicked(Entity*, Qt::MouseButton, RaycastResult*) = 0;
//    virtual void EntityMouseMove(Entity*, Qt::MouseButton, RaycastResult*) = 0;
//    virtual void EntityClickReleased(Entity*, Qt::MouseButton, RaycastResult*) = 0;

protected:

    MenuDataModel *menuDataModel_;
};
