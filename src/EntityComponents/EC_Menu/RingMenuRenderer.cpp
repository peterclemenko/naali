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

#include "Renderer.h"
//#include "OgreRenderingModule.h"
#include "Math/float3.h"
#include "Math/MathConstants.h"
#include "LoggingFunctions.h"

RingMenuRenderer::RingMenuRenderer(EC_MenuContainer *menucontainer, MenuDataModel *datamodel) :
    MenuRendererInterface(datamodel),
    menuContainer_(0),
//    selected_(0),
    scrollerTimer_(0),
    scrollerTimer_Interval(50),
//    ringmesh_(0),
    menuClicked_(false),
    subMenu_(false),
    speed_(0.0)
{
    //LogInfo("RingMenuRenderer constructor");
    menuContainer_ = menucontainer;
    menuContainer_->SetMenuContainerPosition(float3(0,0,-20));
    scrollerTimer_ = new QTimer();
    QObject::connect(scrollerTimer_, SIGNAL(timeout()), this, SLOT(KineticScroller()));

    //LogInfo(ToString(menuDataModel_));
    //LogInfo("number of items in datamodel: "+ToString(menuDataModel_->GetNumberOfDataItems()));
    for (int i=0; i<menuDataModel_->GetNumberOfDataItems();i++)
    {
        //LogInfo("Creating new MenuItem from MenuRenderer");
        EC_MenuItem *menuItem = menuContainer_->CreateMenuItem();
        menuItem->SetDataItem(menuDataModel_->GetMenuDataItem(i));
        MenuItemList_.append(menuItem);
    }

    float phi;
    /// \todo add some logic to determine how large ring we need, based on number of items in it.

    if (menuContainer_->getPhysicsEnabled())
    {
        for (int i = 0; i < MenuItemList_.count(); i++)
        {
            menuContainer_->GetOrCreateRigidBody(MenuItemList_.at(i)->ParentEntity());
        }
    }

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
    delete scrollerTimer_;
}


float3 RingMenuRenderer::CalculateItemPosition(float phi, bool isSelected)
{
    //Vector3df selectedOffset = Vector3df(-1.5, 1.5, 0.0);
    float3 position = float3(0.0, 0.0, 0.0);
    float radius_ = 4.0;
    float item_offset_ = 2.0;

    position.z = radius_ * sin(phi);

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

//    case 2:
//        //vertical
//        //LogInfo("vertical, case 2");
//        position.z = radius_ * cos(phi) - item_offset_;
//        position.x = 0.5*position.y;
//        position += float3(0.0, 0.0, 1.0);
//        break;

//    case 3:
//        //horizontal, sub layer
//        //LogInfo("horizontal, case 3");
//        position.x = radius_ * cos(phi) - item_offset_;
//        position.z = -0.2*position.z;
//        position += float3(2*radius_, -2.0, -0.5);
//        break;
    }

    if(isSelected)
    {
        //position+=selectedOffset;
        return position;
    }
    else
        return position;
}

void RingMenuRenderer::HandleMouseInput(MouseEvent *mouse, RaycastResult *result)
{
//    LogInfo("In virtual HandleMouseInput()");

    if (mouse->IsLeftButtonDown() && !menuClicked_)
    {

        int i=0;
        while (!menuClicked_ && i < MenuItemList_.count())
        {
            if (result->entity == MenuItemList_.at(i)->ParentEntity())
            {
                menuClicked_ = true;
                //to stop scrolling when clicked
                speed_ = 0;
            }
            i++;
        }

    }

    if (menuClicked_ && mouse->IsLeftButtonDown() && mouse->eventType == MouseEvent::MouseMove)
    {

        for (int i = 0; i<MenuItemList_.count(); i++)
        {
            //Sets new angle for components using polar coordinates.
            float phi;
            phi = MenuItemList_.at(i)->getphi() + (float)mouse->RelativeX()/250;

            MenuItemList_.at(i)->setphi(phi);
            //Next position for menu components.
            MenuItemList_.at(i)->SetMenuItemPosition(CalculateItemPosition(phi));

        }
        //LogInfo("Selected planar: " + ToString(selected_));
        speed_=mouse->RelativeX();
    }

    if (mouse->eventType == MouseEvent::MouseReleased && menuClicked_)
    {

        if (speed_ == 0)
        {
            // SUBMENU HANDLER
            if (result->entity == MenuItemList_.at(selected_)->ParentEntity() && !subMenu_)
            {
                if (!MenuItemList_.at(selected_)->OpenSubMenu())
                {

                }

            }

        }

        else if (speed_>3 || speed_<-3)
        {
            if (scrollerTimer_)
            {
                scrollerTimer_->setInterval(scrollerTimer_Interval);
                scrollerTimer_->start();
            }
        }

        menuClicked_ = false;

    }
}

void RingMenuRenderer::KineticScroller()
{
    if (speed_!=0 /*&& !menuIsRotating_*/)
    {
        for (int i=0; i<MenuItemList_.count(); i++)
        {
            float phi = MenuItemList_.at(i)->getphi() + speed_ * scrollerTimer_Interval/10000;
            MenuItemList_.at(i)->setphi(phi);
            MenuItemList_.at(i)->SetMenuItemPosition(CalculateItemPosition(phi));

            if (sin(phi) < -0.970)
            {
                previousSelected_ = selected_;
                selected_=i;
            }
        }

        if (speed_<0)
            speed_ += 1.0;
        else
            speed_ -= 1.0;

        if (speed_ > -1 && speed_ < 1)
            speed_=0;

        //LogInfo("speed: " + ToString(speed_));
    }

    else
    {
        scrollerTimer_->stop();
    }
}
