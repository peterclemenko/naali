/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_MenuItem.cpp
 *  @brief  EC_MenuItem creates single element of 3D Menu component in to scene.
 *  @note   no notes
 */

#include "EC_MenuItem.h"
//#include "EC_MenuContainer.h"

//#include "EC_3DCanvas.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "Entity.h"
//#include "AssetReference.h"

#include "SceneAPI.h"
#include "Framework.h"
#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

EC_MenuItem::EC_MenuItem(Scene *scene) :
    IComponent(scene),
    phi(this,"Phi",0),
    dataitem_(0),
    PivotPlaceable_(0)
{
    bool check;
    // Connect signals from IComponent
    check = connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareMenuItem()), Qt::UniqueConnection);
    Q_ASSERT(check);

    check = connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeChanged(IAttribute*, AttributeChange::Type)));
    Q_ASSERT(check);
}

void EC_MenuItem::PrepareMenuItem()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || framework->IsHeadless())
        return;

}

bool EC_MenuItem::OpenSubMenu()
{
//    if(dataitem_->GetChildCount()>0)
//    {
//        //send signal to someone to open submenu
//        //or create menucontainer by it self...

//        //use dataitem_ as parameter, so it will hide everything above this item from subitems.
//        return true;
//    }
//    else
//    {
//        //do whatever this item should do when it is selected.
//        //for example open browser or so..
        return false;
//    }
}

void EC_MenuItem::SetDataItem(MenuDataItem *dataitemptr)
{
    bool check;
    dataitem_ = dataitemptr;

    check = connect(dataitem_, SIGNAL(DataChanged()), SLOT(UpdateChangedData()));
    Q_ASSERT(check);

    meshreference_ = dataitem_->GetMeshRef();
    for(int i=0; i<dataitem_->GetMaterialRef().count();i++)
    {
        materials_.Append(AssetReference(dataitem_->GetMaterialRef().at(i)));
    }
}

float3 EC_MenuItem::GetMenuItemPosition()
{
    return GetOrCreatePlaceableComponent()->Position();
}

void EC_MenuItem::SetScale(float3 scale)
{
    GetOrCreateMeshComponent()->SetAdjustScale(scale);
}

void EC_MenuItem::SetMenuItemPosition(float3 position)
{
    if (PivotPlaceable_)
    {
//        PivotPlaceable_->SetPosition(position);
        //LogInfo("Moving menuitem, pivotplaceable. Position: "+ToString(position));
    }
    else
    {
        GetOrCreatePlaceableComponent()->SetPosition(position);
        //LogInfo("Moving menuitem");
    }
}

//void EC_MenuItem::SetParentMenuContainer(ComponentPtr parentPlaceable)
//{
//    //setter-function for setting entity position.
//    EC_Placeable *placeable = GetOrCreatePlaceableComponent();
//    if (placeable && parentPlaceable)
//    {
//        /// \todo hack for now..
//        Entity *temp = parentPlaceable.get()->ParentEntity();
////        LogInfo("parent entity ptr: "+ToString(temp));
//        //placeable->SetParent(temp);

//    }
//    else
//        LogError("No placeable or parent placeable available!");
//}


void EC_MenuItem::SetMenuItemVisible()
{
    if (!meshreference_.isEmpty())
    {
        EC_Mesh *mesh = GetOrCreateMeshComponent();
        mesh->SetMeshRef(meshreference_);
        if (!meshreference_.compare("rect_plane.mesh"))
            mesh->SetAdjustOrientation(Quat(0.0, 0.0, 180.0, 0.0));
        if (materials_.Size()>0)
        {
            AttributeChange::Type type = AttributeChange::Default;
            mesh->meshMaterial.Set(materials_, type);
        }
    }
}

EC_Mesh* EC_MenuItem::GetOrCreateMeshComponent()
{
    IComponent *iComponent =  ParentEntity()->GetOrCreateComponent("EC_Mesh", AttributeChange::LocalOnly, false).get();
    //IComponent *iComponent =  ParentEntity()->GetOrCreateComponent("EC_Mesh").get();
    if (iComponent)
    {
        EC_Mesh *mesh = dynamic_cast<EC_Mesh*>(iComponent);
        return mesh;
    }
    else
    {
        LogError("Couldn't get or greate EC_Mesh, returning null pointer");
        return 0;
    }
}

EC_Placeable *EC_MenuItem::GetOrCreatePlaceableComponent()
{
    IComponent *iComponent = ParentEntity()->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false).get();
    //IComponent *iComponent = ParentEntity()->GetOrCreateComponent("EC_Placeable").get();
    if(iComponent)
    {
        EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(iComponent);
        return placeable;
    }
    else
    {
        LogError("Couldn't get or create EC_Placeable");
        return 0;
    }
}

void EC_MenuItem::UpdateChangedData()
{
    meshreference_ = dataitem_->GetMeshRef();
    for (int i=0; i<dataitem_->GetMaterialRef().count();i++)
    {
        materials_.Append(AssetReference(dataitem_->GetMaterialRef().at(i)));
    }
}


void EC_MenuItem::AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType)
{
}
