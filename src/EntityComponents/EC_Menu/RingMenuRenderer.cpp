/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 */

#include "RingMenuRenderer.h"
#include "MenuDataModel.h"
#include "MenuDataItem.h"

#include "EC_MenuContainer.h"
#include "EC_MenuItem.h"
#include "EC_Mesh.h"

#include "Math/float3.h"
#include "Math/MathConstants.h"
#include "LoggingFunctions.h"

RingMenuRenderer::RingMenuRenderer(EC_MenuContainer *menucontainer, MenuDataModel *datamodel) :
    MenuRendererInterface(datamodel),
    menuContainer_(0),
    ringmesh_(0)
{
    LogInfo("RingMenuRenderer constructor");
    menuContainer_ = menucontainer;

    //LogInfo("ActivateMenu()");
    LogInfo(ToString(menuDataModel_));
    LogInfo("number of items in datamodel: "+ToString(menuDataModel_->GetNumberOfDataItems()));
    for (int i=0; i<menuDataModel_->GetNumberOfDataItems();i++)
    {
        qDebug()<<"prkl!!";
        EC_MenuItem *menuItem = menuContainer_->CreateMenuItem();
        menuItem->SetDataItem(menuDataModel_->GetMenuDataItem(i));
        MenuItemList_.append(menuItem);
    }

    float phi;
    /// \todo add some logic to determine how large ring we need, based on number of items in it.

    for (int i = 0; i < MenuItemList_.count(); i++)
    {
        //phi = 2 * float(i) * Ogre::Math::PI / float(MenuItemList_.count()) + ( 0.5*Ogre::Math::PI);
        phi = 2 * float(i) * pi / float(MenuItemList_.count()) - ( 0.5 * pi );
        EC_MenuItem *menuitem = MenuItemList_.at(i);
        //LogInfo(ToString(position));
        //LogInfo("Phi: " + ToString(phi));

        menuitem->setphi(phi);
        menuitem->SetMenuItemPosition(CalculateItemPosition(phi));

        menuitem->SetMenuItemVisible();
    }
}

RingMenuRenderer::~RingMenuRenderer()
{
    LogInfo("RingMenuRenderer destructor");
}

void RingMenuRenderer::Initialize()
{
//    //LogInfo("ActivateMenu()");
//    LogInfo(ToString(menuDataModel_));
//    LogInfo("number of items in datamodel: "+ToString(menuDataModel_->GetNumberOfDataItems()));
//    for (int i=0; i<menuDataModel_->GetNumberOfDataItems();i++)
//    {
//        qDebug()<<"prkl!!";
//        EC_MenuItem *menuItem = menuContainer_->CreateMenuItem();
//        menuItem->SetDataItem(menuDataModel_->GetMenuDataItem(i));
//        MenuItemList_.append(menuItem);
//    }

//    float phi;
//    /// \todo add some logic to determine how large ring we need, based on number of items in it.

//    for (int i = 0; i < MenuItemList_.count(); i++)
//    {
//        //phi = 2 * float(i) * Ogre::Math::PI / float(MenuItemList_.count()) + ( 0.5*Ogre::Math::PI);
//        phi = 2 * float(i) * pi / float(MenuItemList_.count()) - ( 0.5 * pi );
//        EC_MenuItem *menuitem = MenuItemList_.at(i);
//        //LogInfo(ToString(position));
//        //LogInfo("Phi: " + ToString(phi));

//        menuitem->setphi(phi);
//        menuitem->SetMenuItemPosition(CalculateItemPosition(phi));

//        menuitem->SetMenuItemVisible();
//    }

}

float3 RingMenuRenderer::CalculateItemPosition(float phi, bool isSelected)
{
    //Vector3df selectedOffset = Vector3df(-1.5, 1.5, 0.0);
    float3 position = float3(0.0, 0.0, 0.0);
    float radius_ = 4.0;
    float item_offset_ = 2.0;

    position.y = radius_ * sin(phi);

    int menulayer_ = 1;
    switch (menulayer_)
    {
    case 1:
        //horizontal, main layer
        //LogInfo("horizontal, case 1");
        position.x = radius_ * cos(phi) + item_offset_;
        //position.z = 0.5*position.y;
        //position.z = -2.0;
        break;

    case 2:
        //vertical
        //LogInfo("vertical, case 2");
        position.z = radius_ * cos(phi) - item_offset_;
        position.x = 0.5*position.y;
        position += float3(0.0, 0.0, 1.0);
        break;

    case 3:
        //horizontal, sub layer
        //LogInfo("horizontal, case 3");
        position.x = radius_ * cos(phi) - item_offset_;
        position.z = -0.2*position.z;
        position += float3(2*radius_, -2.0, -0.5);
        break;
    }

    if(isSelected)
    {
        //position+=selectedOffset;
        return position;
    }
    else
        return position;
}

//void RingMenuRenderer::EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)
//{
////LogInfo("In virtual EntityClicked()");
//}

//void RingMenuRenderer::EntityMouseMove(Entity*, Qt::MouseButton, RaycastResult*)
//{
////LogInfo("In virtual EntityMouseMove()");
//}

//void RingMenuRenderer::EntityClickReleased(Entity*, Qt::MouseButton, RaycastResult*)
//{
////LogInfo("In virtual EntityClickReleased()");
//}

void RingMenuRenderer::SetMenuPosition()
{
//    Scene::Entity *parent = GetParentEntity();
//    Scene::EntityPtr avatarCameraPtr = parent->GetScene()->GetEntityByName("AvatarCamera");
//    Scene::EntityPtr freeLookCameraPtr = parent->GetScene()->GetEntityByName("FreeLookCamera");

//    //Offset for menu from camera coordinates
//    /// \todo add setter function for these parameters
//    distance.x=0;
//    distance.y=0;
//    distance.z=-22;


//    if (avatarCameraPtr)
//    {
//        Scene::Entity *avatarCamera = avatarCameraPtr.get();
//        cameraPlaceable = dynamic_cast<EC_Placeable*>(avatarCamera->GetComponent("EC_Placeable").get());
//        //follow.Set(true, AttributeChange::LocalOnly);
//        //LogInfo("avatarCamera is active");
//    }
//    else if (freeLookCameraPtr)
//    {
//        Scene::Entity *freeLookCamera = freeLookCameraPtr.get();
//        cameraPlaceable = dynamic_cast<EC_Placeable*>(freeLookCamera->GetComponent("EC_Placeable").get());
//        //LogInfo("freeLookCamera is active");
//    }

//    if (cameraPlaceable)
//    {
//        Transform cameraTransform = cameraPlaceable->gettransform();
//        Vector3df cameraPosition = cameraTransform.position;

//        distance = cameraPlaceable->GetRelativeVector(distance);

//        ownEntityPos.x = cameraPosition.x+distance.x;
//        ownEntityPos.y = cameraPosition.y+distance.y;
//        ownEntityPos.z = cameraPosition.z+distance.z;

//        Transform entityTransform;
//        entityTransform.position=ownEntityPos;

//        //Remove extra 90 degrees which is originally placed to camera's rotation.
//        entityTransform.rotation.x=cameraTransform.rotation.x-90;
//        entityTransform.rotation.y=cameraTransform.rotation.y;
//        entityTransform.rotation.z=cameraTransform.rotation.z;

//        //LogInfo(ToString(entityTransform.position));
//        //LogInfo(ToString(cameraTransform.position));

//        GetOrCreatePlaceableComponent()->settransform(entityTransform);
//    }
//    else
//        LogError("Couldn't get OgreCamera Placeable");

}



void RingMenuRenderer::HandleMouseInput(MouseEvent *event)
{
//    LogInfo("In virtual HandleMouseInput()");
    //mouseinput
//    if (mouse->IsLeftButtonDown() && !menuClicked_)
//    {
//        RaycastResult* result = 0;
//        if (renderer_)
//            result = renderer_->Raycast(mouse->X(), mouse->Y());
//        assert(result);

//        int i=0;
//        while (!menuClicked_ && i < MenuItemList_.count())
//        {
//            if (result->entity_ == MenuItemList_.at(i)->GetParentEntity())
//            {
//                menuClicked_ = true;
//                //to stop scrolling when clicked
//                speed_ = 0;
//            }
//            i++;
//        }
//        if (!startingPositionSaved_)
//        {
//            startingPosition_.setX(mouse->X());
//            startingPosition_.setY(mouse->Y());
//            startingPositionSaved_ = true;
//        }
//    }

//    if (menuClicked_ && mouse->IsLeftButtonDown() && mouse->eventType == MouseEvent::MouseMove)
//    {

//        QObject *qmlmodule = framework_->GetModuleQObj("QMLUIModule");
//        if (qmlmodule)
//        {
//            QObject::connect(this, SIGNAL(SetMouseHandlingInQML(bool)), qmlmodule, SLOT(SetHandleMouseEvents(bool)));
//            SetMouseHandlingInQML(false);
//        }

//        for (int i = 0; i<MenuItemList_.count(); i++)
//        {
//            //Sets new angle for components using polar coordinates.
//            //switch - case could be better way to do this(?)
//            float phi;
//            if (menulayer_%2!=0)
//                phi = MenuItemList_.at(i)->getphi() + (float)mouse->RelativeX()/250;
//            else
//                phi = MenuItemList_.at(i)->getphi() - (float)mouse->RelativeY()/250;

//            MenuItemList_.at(i)->setphi(phi);
//            //Next position for menu components.
//            if (i != selected_)
//            {
//                MenuItemList_.at(i)->SetMenuItemPosition(CalculateItemPosition(phi));
//                if (getPhysicsEnabled())
//                {
//                    EC_RigidBody *rigidbody = GetOrCreateRigidBody(MenuItemList_.at(i)->GetParentEntity());
//                    rigidbody->Activate();
//                }
//            }
//            else
//            {
//                //offset for selected item to popup from other items.
//                MenuItemList_.at(i)->SetMenuItemPosition(CalculateItemPosition(phi, true));
//                if (getPhysicsEnabled())
//                {
//                    EC_RigidBody *rigidbody = GetOrCreateRigidBody(MenuItemList_.at(i)->GetParentEntity());
//                    rigidbody->Activate();
//                }
//            }

//            if (Ogre::Math::Sin(phi) < -0.970)
//            {
//                previousSelected_ = selected_;
//                selected_=i;
//                if (SUBMENUCHANGE)
//                {
//                    if (previousSelected_ != selected_ && subMenu_)
//                    {
//                        CloseSubMenu(previousSelected_);
//                        CreateSubMenu();
//                    }
//                }
//                else
//                {
//                    if (previousSelected_ != selected_ && subMenu_)
//                    {
//                        CloseSubMenu(selected_);
//                        //LogInfo("CloseSubMenu index: "+ToString(previousSelected_));
//                    }
//                }
//            }
//        }
//        //LogInfo("Selected planar: " + ToString(selected_));
//        if (menulayer_%2!=0)
//            speed_=mouse->RelativeX();
//        else
//            speed_=-mouse->RelativeY();
//    }

//    if (mouse->eventType == MouseEvent::MouseReleased && menuClicked_)
//    {
//        QObject *qmlmodule = framework_->GetModuleQObj("QMLUIModule");
//        if (qmlmodule)
//        {
//            SetMouseHandlingInQML(true);
//        }
//        if (speed_ == 0)
//        {
//            RaycastResult* result = 0;
//            if (renderer_)
//                result = renderer_->Raycast(mouse->X(), mouse->Y());
//            assert(result);

//            // SUBMENU HANDLER
//            if (result->entity_ == MenuItemList_.at(selected_)->GetParentEntity() && !subMenu_)
//            {
//                if (!MenuItemList_.at(selected_)->OpenSubMenu())
//                {
//                    if (menulayer_==1)
//                        emit OnMenuSelectionRaw(selected_, 0);
//                    else
//                        emit OnMenuSelection(selected_, 0, this);
//                }
//                else
//                    CreateSubMenu();
//            }
//            else
//            {
//                if (subMenu_)
//                    CloseSubMenu(selected_);

//                int i=0;
//                bool itemFound = false;
//                while (!itemFound && i<MenuItemList_.count())
//                {
//                    if (result->entity_ == MenuItemList_.at(i)->GetParentEntity())
//                        itemFound = true;
//                    else
//                        i++;
//                }
//                if (itemFound)
//                {
//                    switch (menulayer_)
//                    {
//                    case 1:
//                        //horizontal, main layer
//                        if(MenuItemList_.at(i)->GetMenuItemPosition().x<=0)
//                            rotationDirection_=-0.1;
//                        else if(MenuItemList_.at(i)->GetMenuItemPosition().x>=0)
//                            rotationDirection_=0.1;
//                        break;

//                    case 2:
//                        //vertical, sub layer
//                        if(MenuItemList_.at(i)->GetMenuItemPosition().z<=0)
//                            rotationDirection_=-0.1;
//                        else if(MenuItemList_.at(i)->GetMenuItemPosition().z>=0)
//                            rotationDirection_=0.1;

//                        break;

//                    case 3:
//                        //horizontal, sub layer
//                        if(MenuItemList_.at(i)->GetMenuItemPosition().x<=item_offset_)
//                            rotationDirection_=-0.1;
//                        else if(MenuItemList_.at(i)->GetMenuItemPosition().x>=item_offset_)
//                            rotationDirection_=0.1;
//                        break;
//                    }

//                    itemToRotate_ = i;
//                    rotatingTimer_->setInterval(scrollerTimer_Interval);
//                    rotatingTimer_->start();
//                    menuIsRotating_ = true;
//                    //RotateItemToSelected();

//                    itemFound = false;
//                }
//            }
//        }

//        else if (speed_>3 || speed_<-3)
//        {
//            //LogInfo(ToString(speed_));
////            if(subMenu_clicked_)
////                subMenuIsScrolling = true;

//            if (scrollerTimer_)
//            {
//                scrollerTimer_->setInterval(scrollerTimer_Interval);
//                scrollerTimer_->start();
//            }
//        }
//        else if (!menuIsRotating_)
//        {
//            RotateItemToSelected();
//            if(scrollerTimer_)
//                scrollerTimer_->stop();
//            speed_=0;
//        }
//        menuClicked_ = false;
//        //subMenu_clicked_ = false;
//        startingPositionSaved_ = false;
//    }
}
