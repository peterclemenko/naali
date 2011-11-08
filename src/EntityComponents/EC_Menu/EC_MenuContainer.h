/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_MenuContainer.h
 *  @brief  EC_MenuContainer creates 3D Menu in to scene.
 *          It uses MenuDataModel as a data storage and each visible item in menu is created by EC_MenuItem.
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

//#include "Declare_EC.h"

//#include "EC_Placeable.h"

//#include "SceneFwd.h"
//#include "InputFwd.h"

//#include "CoreTypes.h"


#include <QString>
#include <QTimer>
#include <QList>
#include "MenuDataModel.h"

/**
<table class="header">
<tr>
<td>
<h2>EC_MenuContainer</h2>
EC_MenuContainer Component. This component creates subentities with EC_MenuItem component and is parent entity for those.

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

//namespace Scene
//{
//    class Entity;
//}

class IAttribute;
class MenuRendererInterface;
class EC_Placeable;

class EC_MenuItem;

class EC_MenuContainer : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_MenuContainer", 50)

public:
    EC_MenuContainer(Scene *scene);
    ~EC_MenuContainer();

    /// Menu visualization type enumeration
    enum MenuType
    {
        MT_Ring,
        MT_Shelve
    };

    /// Menu type
    Q_PROPERTY(int menuType READ getmenuType WRITE setmenuType)
    DEFINE_QPROPERTY_ATTRIBUTE(int, menuType)

    Q_PROPERTY(bool PhysicsEnabled READ getPhysicsEnabled WRITE setPhysicsEnabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, PhysicsEnabled)

//    Q_PROPERTY(bool runOnLoad READ getrunOnLoad WRITE setrunOnLoad);
//    DEFINE_QPROPERTY_ATTRIBUTE(bool, runOnLoad);

public slots:
    void Initialize();
    void OnAttributeUpdated(IAttribute* attribute);
    void HandleMouseEvent(MouseEvent *);
    void HandleKeyEvent(KeyEvent *);

    void ActivateMenu();

    QObject* GetMenuDataModel();

    EC_MenuItem* CreateMenuItem();
    EC_MenuItem* CreateMenuItem(ComponentPtr parentPlaceable);


private slots:
    void EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);
    void EntityMouseMove(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);
    void EntityClickReleased(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);



    void SetMenuContainerPosition();


private:
    //Framework *framework; ///< Framework.
    InputContextPtr input; ///< Input context.
    MenuDataModel *menuDataModel_;
    MenuRendererInterface *menuRenderer_;

    EC_Placeable* GetOrCreatePlaceableComponent();

};

