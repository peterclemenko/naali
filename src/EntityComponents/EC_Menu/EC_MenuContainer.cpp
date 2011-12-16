/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_MenuContainer.cpp
 *  @brief  EC_MenuContainer creates 3D Menu in to scene.
 *  @note   no notes
 */

#include "StableHeaders.h"

#include "EC_MenuContainer.h"
#include "EC_MenuItem.h"
#include "MenuDataModel.h"
#include "MenuDataItem.h"

#include "MenuRendererInterface.h"
#include "RingMenuRenderer.h"
#include "ShelveMenuRenderer.h"

#include "EC_Placeable.h"
#include "EC_RigidBody.h"
#include "Entity.h"
#include "Scene.h"
#include "Framework.h"

#include "AttributeMetadata.h"
#include "LoggingFunctions.h"

#include "OgreWorld.h"
#include "SceneInteract.h"
#include "SceneAPI.h"
#include "InputAPI.h"

#include "Math/float3.h"

#include <QtGui>

EC_MenuContainer::EC_MenuContainer(Scene *scene) :
    IComponent(scene),
    PhysicsEnabled(this, "Physics for menu", false),
    menuType(this, "menu type", MT_Ring),
    //framework(0),
    menuRenderer_(0),
    menuDataModel_(0)
{
    static AttributeMetadata typeAttrData;
    static bool metadataInitialized = false;
    bool check;
    if (!metadataInitialized)
    {
        typeAttrData.enums[MT_Ring]   = "Ring";
        typeAttrData.enums[MT_Shelve] = "Shelve";
        metadataInitialized = true;
    }
    menuType.SetMetadata(&typeAttrData);

    check = connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Initialize()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
    Q_ASSERT(check);

    ogreWorld = scene->GetWorld<OgreWorld>();

    //framework = GetFramework();
    if (framework)
    {
        input = framework->Input()->RegisterInputContext("MenuContainer", 100);
        input.get()->SetTakeMouseEventsOverQt(true);

        if (input)
        {
            input->SetTakeMouseEventsOverQt(true);
            check = connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
            Q_ASSERT(check);
        }
    }
}

EC_MenuContainer::~EC_MenuContainer()
{
    //delete menuRenderer_;
    delete menuDataModel_;
}

void EC_MenuContainer::Initialize()
{
    //LogInfo("Container datamodelPtr: "+ToString(menuDataModel_));
    if (!menuDataModel_)
    {
        menuDataModel_ = new MenuDataModel();
        //LogInfo("Container datamodelPtr1: "+ToString(menuDataModel_));
    }
}

void EC_MenuContainer::ActivateMenu()
{
    if (!menuRenderer_)
    {
        switch(menuType.Get())
        {
        case MT_Ring:
            menuRenderer_ = new RingMenuRenderer(this, menuDataModel_);
            //LogInfo("Container datamodelPtr2: "+ToString(menuDataModel_));
            //LogInfo("items in datamodel (menucontainer): " + ToString(menuDataModel_->GetNumberOfDataItems()));
            break;

        case MT_Shelve:
            menuRenderer_ = new ShelveMenuRenderer(menuDataModel_);
            break;

        default:
            LogError("Invalid menutype!");
            break;
        }
    }
}

void EC_MenuContainer::SetMenuContainerPosition(float3 distance)
{
    float3 ownEntityPos;
    EC_Placeable *cameraPlaceable=0;

    Scene *scene = GetFramework()->Scene()->MainCameraScene();

    EntityPtr avatarCameraPtr = scene->GetEntityByName("AvatarCamera");
    EntityPtr freeLookCameraPtr = scene->GetEntityByName("FreeLookCamera");
    //Offset for menu from camera coordinates


    if (avatarCameraPtr)
    {
        Entity *avatarCamera = avatarCameraPtr.get();
        cameraPlaceable = dynamic_cast<EC_Placeable*>(avatarCamera->GetComponent("EC_Placeable").get());
        //follow.Set(true, AttributeChange::LocalOnly);
        LogInfo("avatarCamera is active");
    }
    else if (freeLookCameraPtr)
    {
        Entity *freeLookCamera = freeLookCameraPtr.get();
        cameraPlaceable = dynamic_cast<EC_Placeable*>(freeLookCamera->GetComponent("EC_Placeable").get());
        LogInfo("freeLookCamera is active");
        LogInfo(ToString(cameraPlaceable->WorldPosition()));
    }

    distance = cameraPlaceable->WorldOrientation() * distance;

    if (cameraPlaceable)
    {
        Transform cameraTransform = cameraPlaceable->gettransform();
        float3 cameraPosition = cameraTransform.pos;

        //distance = cameraPlaceable->GetRelativeVector(distance);

        ownEntityPos.x = cameraPosition.x+distance.x;
        ownEntityPos.y = cameraPosition.y+distance.y;
        ownEntityPos.z = cameraPosition.z+distance.z;

        Transform entityTransform;
        entityTransform.SetPos(ownEntityPos.x, ownEntityPos.y, ownEntityPos.z);

        //Remove extra 90 degrees which is originally placed to camera's rotation.
        //entityTransform.SetRotation(cameraTransform.rot.x-90, cameraTransform.rot.y, cameraTransform.rot.z);

        LogInfo("entityTransform "+entityTransform.toString());
        LogInfo("cameraTransform "+cameraTransform.toString());

        GetOrCreatePlaceableComponent()->settransform(entityTransform);
    }
    else
        LogError("Couldn't get OgreCamera Placeable");
}

QObject* EC_MenuContainer::GetMenuDataModel()
{
    //LogInfo("GetMenuDataModel - before");
    if (menuDataModel_)
    {
        //LogInfo("GetMenuDataModel - if" +ToString(menudatamodel_));
        return menuDataModel_;
    }
    else
    {
        //LogInfo("GetMenuDataModel - else" +ToString(menudatamodel_));
        menuDataModel_ = new MenuDataModel();
        return menuDataModel_;
    }
}

EC_MenuItem* EC_MenuContainer::CreateMenuItem()
{
    ComponentPtr parent = ParentEntity()->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false);
    //ComponentPtr parent = ParentEntity()->GetOrCreateComponent("EC_Placeable");
    return CreateMenuItem(parent);
}

EC_MenuItem* EC_MenuContainer::CreateMenuItem(ComponentPtr parentPlaceable)
{
    Scene *scene = GetFramework()->Scene()->MainCameraScene();
    EntityPtr entity_ = scene->CreateEntity(scene->NextFreeIdLocal(), QStringList(), AttributeChange::LocalOnly, false);
    //EntityPtr entity_ = scene->CreateEntity(scene->NextFreeId(), QStringList());

    //LogInfo("Pointer " + ToString(entity_));
    if (!entity_)
        LogError("Couldn't create entity with given ID");
    else
    {
        Entity *MenuItemEntity = entity_.get();

        EC_Placeable *itemPlaceable = GetOrCreatePlaceableComponent(MenuItemEntity);
        itemPlaceable->SetParent(parentPlaceable.get()->ParentEntity(), false);

        IComponent *iComponent = MenuItemEntity->GetOrCreateComponent("EC_MenuItem", AttributeChange::LocalOnly, false).get();
        //IComponent *iComponent = MenuItemEntity->GetOrCreateComponent("EC_MenuItem").get();

        if (iComponent)
        {
            EC_MenuItem *menuItem = dynamic_cast<EC_MenuItem*>(iComponent);

            //Sets parent entity for menuItem-entitys placeable component
            //menuItem->SetParentMenuContainer(parentPlaceable);
            scene->EmitEntityCreated(MenuItemEntity, AttributeChange::LocalOnly);
            //scene->EmitEntityCreated(MenuItemEntity, AttributeChange::Default);

            return menuItem;
        }
        else
        {
            LogError("Error while creating MenuItem");
        }
    }
    return 0;
}

void EC_MenuContainer::GetOrCreateRigidBody(Entity *entity)
{
    if (entity)
    {
        entity->GetOrCreateComponent("EC_RigidBody", AttributeChange::LocalOnly, false).get();
        //entity->GetOrCreateComponent("EC_RigidBody").get();
    }
    else
        LogError("Invalid entity pointer!");
}

void EC_MenuContainer::HandleMouseEvent(MouseEvent *event)
{
    if (event->IsLeftButtonDown())
    {
        IRenderer *renderer = framework->Renderer();
        if (!renderer)
            return;

        RaycastResult *result = renderer->Raycast(event->x, event->y);
        if (!result)
            return;

        if (menuRenderer_)
            menuRenderer_->HandleMouseInput(event, result);
        else
            LogError("Error while trying to handle mouseinput, MenuRenderer not found!");
    }
}

EC_Placeable* EC_MenuContainer::GetOrCreatePlaceableComponent()
{
    GetOrCreatePlaceableComponent(ParentEntity());
}

EC_Placeable* EC_MenuContainer::GetOrCreatePlaceableComponent(Entity *entity)
{
    if (!entity)
            return 0;
    IComponent *iComponent = entity->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false).get();
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(iComponent);

    return placeable;
}

void EC_MenuContainer::OnAttributeUpdated(IAttribute* attribute)
{
    if (attribute == &PhysicsEnabled)
    {
        if (PhysicsEnabled.Get() == true)
        {

        }

        else
        {

        }
    }

    if (attribute == &menuType)
    {
//        if (menuRenderer_)
//        {
//            delete menuRenderer_;
//            menuRenderer_ = 0;
//        }

//        switch(menuType.Get())
//        {
//        case MT_Ring:
//            menuRenderer_ = new RingMenuRenderer(this, menuDataModel_);
//            break;

//        case MT_Shelve:
//            menuRenderer_ = new ShelveMenuRenderer(menuDataModel_);
//            break;

//        }
    }
}


