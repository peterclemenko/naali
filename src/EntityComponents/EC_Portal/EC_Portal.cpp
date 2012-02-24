#include "EC_Portal.h"

#include "Framework.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "FrameAPI.h"
#include "SceneAPI.h"
#include "EntityAction.h"

#include "TundraLogicModule.h"
#include "Client.h"

#include "LoggingFunctions.h"


EC_Portal::EC_Portal(Scene *scene) :
    IComponent(scene),
    address(this, "Address:"),
    port(this, "Port:"),
    protocol(this, "Protocol:"),
    position_(0,0,0),
    sceneInteract_(framework->Scene()->GetSceneInteract()),
    scene_(scene),
    parent_(ParentEntity())
{
    connect(sceneInteract_, SIGNAL(EntityClicked(Entity*,Qt::MouseButton,RaycastResult*)), this, SLOT(parentClicked(Entity*,Qt::MouseButton)));
    connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(Update(float)));
}

EC_Portal::~EC_Portal()
{
}

void EC_Portal::Update(float frametime)
{
    parent_ = ParentEntity();
    scene_ = parent_->ParentScene();
}

void EC_Portal::parentClicked(Entity *ent, Qt::MouseButton button)
{
    if (ent != parent_)
    {
        return;
    }
    if (button != Qt::LeftButton)
    {
        return;
    }
    if (scene_->Name() != framework->Scene()->MainCameraScene()->Name())
    {
        return;
    }

    // Get placeable for portal position in 3D-space.
    EC_Placeable* placeable = parent_->GetComponent<EC_Placeable>().get();
    if (placeable)
    {
        position_ = placeable->transform.Get().pos;
    }

    TundraLogic::TundraLogicModule* tundra = framework->GetModule<TundraLogic::TundraLogicModule>();
    TundraLogic::Client *client = tundra->GetClient().get();

    EntityPtr avatar = scene_->GetEntityByName("Avatar" + QString::number(client->GetConnectionID()));
    if (avatar)
    {
        EC_Placeable* placeable = avatar->GetComponent<EC_Placeable>().get();
        if (placeable)
        {
            float3 avatarPos = placeable->transform.Get().pos;
            float distance = avatarPos.Distance(position_);
            ::LogInfo("Avatar distance: " + QString::number(distance));
            if (distance < 3)
            {
                LogInfo("Connection initiated from portal!\n");
                client->Login(address.Get(), port.Get().toInt(),"PortalMan","", protocol.Get());
            }
        }
    }
    else
    {
        EntityPtr camera = scene_->GetEntityByName("FreeLookCamera");
        if (camera)
        {
            EC_Placeable *placeable = camera->GetComponent<EC_Placeable>().get();
            if (placeable)
            {
                float3 cameraPos = placeable->transform.Get().pos;
                float distance = cameraPos.Distance(position_);
                ::LogInfo("Camera distance: " + QString::number(distance));
                if (distance < 9)
                {
                    LogInfo("Connection initiated from portal!\n");
                    client->Login(address.Get(), port.Get().toInt(),"PortalMan","", protocol.Get());
                }

            }
        }
    }
}
