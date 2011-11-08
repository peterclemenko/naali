/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_MenuItem.h
 *  @brief  EC_MenuItem is part of 3Dmenu and it is used by EC_MenuContainer.
 *  @note   no notes
 *
 */

#pragma once

#include "IComponent.h"
#include "AssetReference.h"
#include "AssetFwd.h"

#include "FrameworkFwd.h"
#include "SceneFwd.h"
#include "InputFwd.h"

#include "Math/float3.h"

#include "MenuDataItem.h"




/**
<table class="header">
<tr>
<td>
<h2>EC_MenuItem</h2>
EC_MenuItem Component.

Registered by RexLogic::RexLogicModule.

<b>No Attributes.</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

</table>
*/

class EC_Mesh;
class EC_3DCanvas;
class EC_Placeable;
class MenuDataModel;

class EC_MenuItem : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_MenuItem", 51)

public:
    EC_MenuItem(Scene *scene);

    Q_PROPERTY(float phi READ getphi WRITE setphi)
    DEFINE_QPROPERTY_ATTRIBUTE(float, phi)

private:
    int widgetSubmesh_;

    QString meshreference_;
    AssetReferenceList materials_;

    MenuDataItem *dataitem_;
    EC_Placeable *PivotPlaceable_;

public slots:
    bool OpenSubMenu();

    void SetDataItem(MenuDataItem *dataitemptr);

    //! Returns MenuDataItem related to this EC_MenuItem
    MenuDataItem* GetDataItem() { return dataitem_; }

    //! Returns position data from EC_Placeable in same entity
    float3 GetMenuItemPosition();

    //! Set scale for EC_Mesh attached in same entity
    void SetScale(float3 scale);

    //! Setter for entity position
    void SetMenuItemPosition(float3 position);

//    void SetPivotPlaceable(EC_Placeable* placeable, ComponentPtr parentPlaceable);

    void SetMenuItemVisible();
    void SetMenuItemHidden();

    void SetMenuItemWidget(QWidget *data, int subMeshIndex = 0);

    //! Setter for EC_Placeable parameters
    void SetParentMenuContainer(ComponentPtr);

    //! Get parent entitys EC_Mesh component or create one if not present.
    EC_Mesh* GetOrCreateMeshComponent();

    //! Get parent entitys EC_Placeable component or create one if not present.
    EC_Placeable* GetOrCreatePlaceableComponent();


private slots:
    //! Prepares everything related to the parent widget and other needed components.
    void PrepareMenuItem();

    //! Monitors this components Attribute changes.
    void AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType);

    //! Setter for what mesh to use in this menuitem
    void SetMenuItemMesh(QString, QStringList);

    //! Monitors data changes in attached MenuDataItem
    void UpdateChangedData();

    //! Get parent entitys EC_3DCanvas component or create one if not present.
    //EC_3DCanvas* GetOrCreateCanvasComponent();

signals:
    void OnAttributeChanged(IAttribute*, AttributeChange::Type);

};

