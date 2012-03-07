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
#include <pcl/common/time.h>

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
    polygon_mesh_ = inputMesh;
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

    double startime = pcl::getTime();

    //ogreManual_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    //ogreManual_->setUseIdentityProjection(true);
    //ogreManual_->setUseIdentityView(true);
    //ogreManual_->setQueryFlags(0);

    // compute the correct number of values:
    size_t triangle_size = input_cloud_->points.size();

    ogreManual_->estimateVertexCount(triangle_size);
    LogInfo("Triangle size: "+ ToString(triangle_size));

    LogInfo("MeshConverter: Begin of manual object creation");
    ogreManual_->clear();
    ogreManual_->begin("CapturedObject", Ogre::RenderOperation::OT_TRIANGLE_LIST);//OT_TRIANGLE_STRIP

    LogInfo("MeshConverter: Setting positions...");

    for (size_t i = 0; i < input_cloud_->points.size(); i++)
    {
        Ogre::Real r = (Ogre::Real)input_cloud_->points[i].r / (Ogre::Real)255;
        Ogre::Real g = (Ogre::Real)input_cloud_->points[i].g / (Ogre::Real)255;
        Ogre::Real b = (Ogre::Real)input_cloud_->points[i].b / (Ogre::Real)255;

        ogreManual_->colour(r, g, b);
        ogreManual_->position(input_cloud_->points[i].x, input_cloud_->points[i].y, input_cloud_->points[i].z);
        ogreManual_->normal(input_cloud_->points[i].data_c[0], input_cloud_->points[i].data_c[1], input_cloud_->points[i].data_c[2]);
    }

    for (size_t i = 0; i < polygon_mesh_->polygons.size(); i++)
    {
        for(size_t j = 0; j < polygon_mesh_->polygons[i].vertices.size(); j++)
        {
            int index = polygon_mesh_->polygons[i].vertices[j];
            ogreManual_->index(index);
        }
    }
    ogreManual_->end();

    LogInfo("MeshConverter: Object created!");
    double objectCreateTime = pcl::getTime() - startime;
    LogInfo("Time needed for custom object creation: " + ToString(objectCreateTime));

    LogInfo("MeshConverter: create EC_OgreCustomObject");
    EntityPtr entity = scene_->CreateEntity(scene_->NextFreeIdLocal(), QStringList(), AttributeChange::LocalOnly, false);
    ComponentPtr componentPtr = entity->GetOrCreateComponent("EC_OgreCustomObject", AttributeChange::LocalOnly, false);
    EC_OgreCustomObject *customObject = dynamic_cast<EC_OgreCustomObject*>(componentPtr.get());

    customObject->SetName("ImportedMesh");
    customObject->CommitChanges(ogreManual_);

    // Get EC_Placeable for custom ombject
    componentPtr = entity->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false);
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(componentPtr.get());
    placeable->SetTransform(Quat(1.0, 0.0, 0.0, 0.0), float3(0.0, 1.0, -30.0), float3(10.0, 10.0, 10.0));

    customObject->SetPlaceable(componentPtr);
}
} // end of namespace ObjectCapture
