/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#pragma once

#include "InputAPI.h"
#include "MenuDataModel.h"
class Entity;
class RaycastResult;

class MenuRendererInterface : public QObject
{

public:
    MenuRendererInterface(MenuDataModel *datamodel){menuDataModel_ = datamodel;}
    virtual ~MenuRendererInterface() = 0;
    virtual void HandleMouseInput(MouseEvent*, RaycastResult* result) = 0;

protected:
    MenuDataModel *menuDataModel_;
};
