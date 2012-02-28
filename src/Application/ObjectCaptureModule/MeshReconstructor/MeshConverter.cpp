#include "MeshConverter.h"
#include "ObjectCaptureModuleDefines.h"

#include "EC_OgreCustomObject.h"
#include "Ogre.h"
#include "OgreWorld.h"
#include "OgreManualObject.h"

#include "IModule.h"
#include "OgreRenderingModule.h"
#include "Framework.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"
#include "EC_Placeable.h"

#include "LoggingFunctions.h"

namespace ObjectCapture
{

MeshConverter::MeshConverter(IModule *module) :
    module_(module)
{

}
MeshConverter::~MeshConverter()
{

}

void MeshConverter::Create(pcl::PolygonMesh::Ptr inputMesh, pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr inputCloud)
{
    input_mesh_ = inputMesh;
    input_cloud_ = inputCloud;

    this->createMesh();
}

void MeshConverter::createMesh()
{
    Framework *framework = module_->GetFramework();
    scene_ = framework->Scene()->MainCameraScene();
    if (scene_)
        world_ = scene_->GetWorld<OgreWorld>();
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    ogreManual_ = sceneMgr->createManualObject(world->GetUniqueObjectName("ImportedMesh"));

    //ogreManual_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    //ogreManual_->setUseIdentityProjection(true);
    //ogreManual_->setUseIdentityView(true);
    //ogreManual_->setQueryFlags(0);

    LogInfo("MeshConverter: Begin of manual object creation");
    ogreManual_->clear();
    ogreManual_->begin("CapturedObject", Ogre::RenderOperation::OT_TRIANGLE_LIST);//OT_TRIANGLE_STRIP

    LogInfo("MeshConverter: Setting positions...");
    unsigned int i = 0;
    for (size_t loop1 = 0; loop1 < input_mesh_->polygons.size(); loop1++)
    {
        for(size_t loop2 = 0; loop2 < input_mesh_->polygons[loop1].vertices.size(); loop2++)
        {
            int index = input_mesh_->polygons[loop1].vertices[loop2];

            Ogre::Real r = (Ogre::Real)input_cloud_->points[index].r / (Ogre::Real)255;
            Ogre::Real g = (Ogre::Real)input_cloud_->points[index].g / (Ogre::Real)255;
            Ogre::Real b = (Ogre::Real)input_cloud_->points[index].b / (Ogre::Real)255;

            ogreManual_->colour(r, g, b);
            ogreManual_->position(input_cloud_->points[index].x, input_cloud_->points[index].y, input_cloud_->points[index].z);
            ogreManual_->normal(input_cloud_->points[index].data_c[0], input_cloud_->points[index].data_c[1], input_cloud_->points[index].data_c[2]);
            ogreManual_->index(i);
            i++;
        }
    }    
    ogreManual_->end();
    LogInfo("MeshConverter: Object created!");

    //ogreManual_->setVisible(true);

    LogInfo("MeshConverter: create EC_OgreCustomObject");
    EntityPtr entity = scene_->CreateEntity();
    ComponentPtr componentPtr = entity->GetOrCreateComponent("EC_OgreCustomObject");
    EC_OgreCustomObject *customObject = dynamic_cast<EC_OgreCustomObject*>(componentPtr.get());

    customObject->SetName("ImportedMesh");
    customObject->CommitChanges(ogreManual_);

    // Get EC_Placeable for custom ombject
    componentPtr = entity->GetOrCreateComponent("EC_Placeable");
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(componentPtr.get());
    placeable->SetTransform(Quat(1.0, 0.0, 0.0, 0.0), float3(0.0, 1.0, -30.0), float3(10.0, 10.0, 10.0));

    customObject->SetPlaceable(componentPtr);
}
} // end of namespace ObjectCapture
