// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ObjectCaptureModule.h"
#include <pcl/io/pcd_io.h>
#include "CloudProcessor/CloudProcessor.h"
#include "MeshReconstructor/MeshReconstructor.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "IComponent.h"

#include "Entity.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{
ObjectCaptureModule::ObjectCaptureModule() :
    IModule("ObjectCapture"),
    cloud_processor_(new CloudProcessor()),
    mesh_reconstructor_(new MeshReconstructor())
{
    bool check;

    check = connect(cloud_processor_, SIGNAL(RGBUpdated(QImage)), this, SIGNAL(previewFrameUpdated(QImage)));
    Q_ASSERT(check);

    check = connect(cloud_processor_, SIGNAL(registrationFinished()), this, SLOT(registrationFinished()));
    Q_ASSERT(check);

    check = connect(mesh_reconstructor_, SIGNAL(cloudProcessingFinished()), this, SLOT(cloudProcessingFinished()));
    Q_ASSERT(check);
}

ObjectCaptureModule::~ObjectCaptureModule()
{
    SAFE_DELETE(cloud_processor_);
    SAFE_DELETE(mesh_reconstructor_)
}

void ObjectCaptureModule::Load()
{
    framework_->RegisterDynamicObject("ObjectCapture", this);
}

void ObjectCaptureModule::Initialize()
{
}

void ObjectCaptureModule::Uninitialize()
{
}

void ObjectCaptureModule::Update(f64 frametime)
{
}

void ObjectCaptureModule::startCapturing()
{
    cloud_processor_->startCapture();
}

void ObjectCaptureModule::stopCapturing()
{
    cloud_processor_->stopCapture();
}

void ObjectCaptureModule::captureCloud()
{
    cloud_processor_->captureCloud();
}

void ObjectCaptureModule::finalizeCapturing()
{
    cloud_processor_->registerClouds();
}

void ObjectCaptureModule::registrationFinished()
{
    PointCloud::Ptr captured_cloud;
    if(captured_cloud.get())
    {
        // pass for meshreconstructor
        //LogInfo("setInputCloud()");
        mesh_reconstructor_->setInputCloud(captured_cloud);
        mesh_reconstructor_->processCloud();
    }
}

void ObjectCaptureModule::cloudProcessingFinished()
{
    //Add finalized mesh to scene
    Scene *scene = framework_->Scene()->MainCameraScene();

    EntityPtr entity_ = scene->CreateEntity(scene->NextFreeIdLocal(), QStringList(), AttributeChange::LocalOnly, false);
    if (!entity_)
        LogError("ObjectCapture: Error while creating entity for mesh");
    else
    {
        Entity *meshEntity = entity_.get();
        IComponent *iComponent = meshEntity->GetOrCreateComponent("EC_Mesh", AttributeChange::LocalOnly, false).get();
        if (iComponent)
        {
            EC_Mesh *mesh = dynamic_cast<EC_Mesh*>(iComponent);
            mesh->SetMesh("testmesh.mesh");
        }

        iComponent = meshEntity->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false).get();
        if (iComponent)
        {
            EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(iComponent);
            placeable->setvisible(false);
        }

        scene->EmitEntityCreated(meshEntity, AttributeChange::LocalOnly);
        emit objectCaptured(meshEntity->Id());
    }
}

}// end of namespace: ObjectCapture

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new ObjectCapture::ObjectCaptureModule();
        fw->RegisterModule(module);
    }
}
