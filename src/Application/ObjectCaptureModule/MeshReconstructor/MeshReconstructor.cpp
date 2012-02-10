// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MeshReconstructor.h"

#include <pcl/surface/reconstruction.h>
#include <pcl/surface/surfel_smoothing.h>
#include <pcl/surface/mls.h>

#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/vtk_io.h>
#include "pcl/common/common_headers.h"

#include "pcl/features/normal_3d.h"
#include <pcl/point_traits.h>

#include <boost/thread/thread.hpp>

#include <pcl/surface/gp3.h>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

namespace ObjectCapture
{

MeshReconstructor::MeshReconstructor() :
    point_cloud_(new PointCloud()),
    normals_(new SurfaceNormals()),
    smoothed_cloud_(new PointCloud())
{

}

MeshReconstructor::~MeshReconstructor()
{

}

void MeshReconstructor::processCloud(PointCloud::Ptr cloud)
{
    point_cloud_ = cloud;
    LogInfo("Performing MovingLeastSquares smoothing");
    MovingLeastSquares();
    //LogInfo("NormalEstimation()");
    //NormalEstimation();
    LogInfo("GreedyProjection()");
    GreedyProjection_Mesher();
    LogInfo("convertVtkToMesh()");
    convertVtkToMesh();

    emit cloudProcessingFinished();
}

void MeshReconstructor::convertVtkToMesh()
{
    char Command[100];

    sprintf(Command, "python OgreMeshXML.py -i testmesh.vtk -o testmesh.xml");
    if (system(Command))
        LogInfo("Error while converting vtk to xml!\n");

    sprintf(Command, "OgreXMLConverter testmesh.xml testmesh.mesh");
    if (system(Command))
        LogInfo("Error while converting xml to mesh!\n");

    sprintf(Command, "mv testmesh.mesh data/assets/");
    if (system(Command))
        LogInfo("Error while moving mesh to scenes folder!\n");
}

void MeshReconstructor::MovingLeastSquares()
{
    // Create a KD-Tree
    pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZRGB>(false));
    //pcl::search::Search<PointCloud>::Ptr tree (new pcl::search::Search<PointCloud>(false));

    // Init object (second point type is for the normals, even if unused)
    pcl::MovingLeastSquares<pcl::PointXYZRGB, pcl::Normal> mls;

    // Optionally, a pointer to a cloud can be provided, to be set by MLS
    mls.setOutputNormals (normals_);

    // Set parameters
    mls.setInputCloud (point_cloud_);
    mls.setPolynomialFit (true);
    mls.setPolynomialOrder(2);
    mls.setSearchMethod (tree);
    mls.setSearchRadius (0.03);
    mls.setSqrGaussParam(0.0009);

    // Reconstruct
    LogInfo("starting MovingLeastSquare reconstruction");
    mls.reconstruct (*smoothed_cloud_);
}

void MeshReconstructor::NormalEstimation()
{
    float normal_search_radius = 0.0025;

    //initialize normal estimation
    pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> normal_estimation;
    normal_estimation.setInputCloud (smoothed_cloud_);

    //initialize search tree
    pcl::search::KdTree<pcl::PointXYZRGB>::Ptr search_tree (new pcl::search::KdTree<pcl::PointXYZRGB>());
    search_tree->setInputCloud (smoothed_cloud_);

    normal_estimation.setSearchMethod (search_tree);
    //normal_estimation.setKSearch (20); //if we want more accurate results, re-run with KSearch!
    normal_estimation.setRadiusSearch (normal_search_radius);

    normal_estimation.compute (*normals_);
}

void MeshReconstructor::GreedyProjection_Mesher()
{
    // beging of meshing
    pcl::PolygonMesh output;

    LogInfo("Meshing");
    pcl::GreedyProjectionTriangulation<pcl::PointXYZRGBNormal> mesher;
    pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointXYZRGBNormal>());
    pcl::concatenateFields(*smoothed_cloud_, *normals_, *cloud_with_normals);
    mesher.setInputCloud(cloud_with_normals);

    //std::cout << "Setting search method" << std::endl;
    pcl::search::KdTree<pcl::PointXYZRGBNormal>::Ptr tree2 (new pcl::search::KdTree<pcl::PointXYZRGBNormal>());
    mesher.setSearchMethod(tree2);//pcl::search::Search<pcl::PointNormal>::Ptr()

    //std::cout << "Setting search radius" << std::endl;
    mesher.setSearchRadius(0.05);
    //mesher.setConsistentVertexOrdering(true); // Requires PCL-1.5 or higher!

    mesher.setMu(2.5);
    mesher.setMaximumNearestNeighbors(1000);
    mesher.setMaximumSurfaceAngle(M_PI/4);
    mesher.setMinimumAngle(M_PI/18); // 10 degrees
    mesher.setMaximumAngle(2*M_PI/3); // 120 degrees
    mesher.setNormalConsistency(false);

    std::cout << "Reconstructing" << std::endl;
    mesher.reconstruct(output);

    std::stringstream ssp;
    ssp << "testmesh.vtk";

    LogInfo( "Saving file: " + ssp.str());
    pcl::io::saveVTKFile(ssp.str(), output);
}
} //end of namespace ObjectCapture

