// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once
#include <QObject>

#include "ObjectCaptureModuleDefines.h"
#include <pcl/point_types.h>
#include <pcl/pcl_base.h>
#include <pcl/PolygonMesh.h>

class IModule;

namespace ObjectCapture
{

class MeshReconstructor : public QObject
{
    Q_OBJECT

public:
    MeshReconstructor();
    ~MeshReconstructor();

public slots:
    void processCloud(PointCloud::Ptr cloud);
    void convertVtkToMesh();

private slots:
    void MovingLeastSquares();
    void NormalEstimation();

    /// Create mesh with greedy projection
    /// @param outputCloud Boost pointer to output cloud
    void GreedyProjection_Mesher();

    void ConvertPolygonsToCollada();

signals:
    void cloudProcessingFinished(pcl::PolygonMesh::Ptr, pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr);

private:
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloud_;
    SurfaceNormals::Ptr normals_;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr smoothed_cloud_;
    pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr cloud_with_normals_;
    pcl::PolygonMesh::Ptr polygonMesh_;

};
} //end of namespace ObjectCapture
