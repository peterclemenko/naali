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
    sceneInteract_(framework->Scene()->GetSceneInteract())
{
    //connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateMethod()));
    connect(sceneInteract_, SIGNAL(EntityClicked(Entity*,Qt::MouseButton,RaycastResult*)), this, SLOT(parentClicked(Entity*,Qt::MouseButton)));
}

EC_Portal::~EC_Portal()
{
}

void EC_Portal::UpdateMethod()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        FrameAPI* frame = framework->Frame();

        EC_Portal *portal = parent->GetComponent<EC_Portal>().get();
        if (portal)
        {
            LogWarning("Another Portal exists in this entity.");
        }
    }
}

void EC_Portal::Update(float frametime)
{

}

void EC_Portal::parentClicked(Entity *ent, Qt::MouseButton button)
{
    if (!(ent == ParentEntity()))
        return;
    if (button != Qt::LeftButton)
    {
        return;
    }
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    Scene *scene = parent->ParentScene();
    if (!scene)
        return;
    if (!(scene->Name() == framework->Scene()->MainCameraScene()->Name()))
        return;

    // Get placeable for portal position in 3D-space.
    EC_Placeable* placeable = parent->GetComponent<EC_Placeable>().get();
    if (placeable)
    {
        position_ = placeable->transform.Get().pos;
    }

    TundraLogic::TundraLogicModule* tundra = framework->GetModule<TundraLogic::TundraLogicModule>();
    TundraLogic::Client *client = tundra->GetClient().get();

    EntityPtr avatar = scene->GetEntityByName("Avatar" + QString::number(client->GetConnectionID()));
    if (avatar)
    {
        EC_Placeable* placeable = avatar->GetComponent<EC_Placeable>().get();
        if (placeable)
        {
            float3 avatarPos = placeable->transform.Get().pos;
            float distance = avatarPos.Distance(position_);
            ::LogInfo("Distance: " + QString::number(distance));
            if (distance < 3)
            {
                LogInfo("Connection initiated from portal!\n");
                client->Login(address.Get(), port.Get().toInt(),"portal","portal", protocol.Get());
            }
        }
    }
    else
    {
        EntityPtr camera = scene->GetEntityByName("FreeLookCamera");
        if (camera)
        {
            EC_Placeable *placeable = camera->GetComponent<EC_Placeable>().get();
            if (placeable)
            {
                float3 cameraPos = placeable->transform.Get().pos;
                float distance = cameraPos.Distance(position_);
                ::LogInfo("Distance: " + QString::number(distance));
                if (distance < 9)
                {
                    LogInfo("Connection initiated from portal!\n");
                    client->Login(address.Get(), port.Get().toInt(),"portal","portal", protocol.Get());
                    FrameAPI* frame = framework->Frame();
                }

            }
        }
    }
}
