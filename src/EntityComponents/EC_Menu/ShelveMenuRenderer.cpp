/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#include "ShelveMenuRenderer.h"
#include "MenuDataModel.h"
#include "MenuDataItem.h"

#include "LoggingFunctions.h"

ShelveMenuRenderer::ShelveMenuRenderer(MenuDataModel *datamodel):MenuRendererInterface(datamodel)
{
    LogInfo("ShelveMenuRenderer constructor");
}

ShelveMenuRenderer::~ShelveMenuRenderer()
{
    LogInfo("ShelveMenuRenderer destructor");
}

void ShelveMenuRenderer::EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)
{

}

void ShelveMenuRenderer::EntityMouseMove(Entity*, Qt::MouseButton, RaycastResult*)
{

}

void ShelveMenuRenderer::EntityClickReleased(Entity*, Qt::MouseButton, RaycastResult*)
{

}

void ShelveMenuRenderer::SetMenuPosition()
{

}

void ShelveMenuRenderer::HandleMouseInput(MouseEvent *event)
{
    LogInfo("In virtual HandleMouseInput()");
    //mouseinput

}
