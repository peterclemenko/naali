#pragma once

#include <QObject>

#include "ObjectCaptureModuleDefines.h"

#include "pcl/PolygonMesh.h"
#include "OgreModuleFwd.h"

class IModule;
class Scene;

namespace ObjectCapture
{

class MeshConverter : public QObject
{
    Q_OBJECT

public:
    MeshConverter(IModule*);
    ~MeshConverter();

public slots:
    void Create(pcl::PolygonMesh::Ptr inputMesh, pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr inputCloud);

private slots:
    void createMesh();

private:
    pcl::PolygonMesh::Ptr polygon_mesh_;
    pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr input_cloud_;

    IModule *module_;
    OgreWorldWeakPtr world_;
    Scene *scene_;

    Ogre::ManualObject *ogreManual_;

};

} // end of namespace ObjectCapture
