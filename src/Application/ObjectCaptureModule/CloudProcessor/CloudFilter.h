// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/search/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/filters/extract_indices.h>

#include <QObject>

namespace ObjectCapture
{

class CloudFilter : public QObject
{
    Q_OBJECT

public:
    CloudFilter();
    ~CloudFilter();

    /// Filters cloud by given depth range
    PointCloud::Ptr filterDepth(PointCloud::ConstPtr cloud, float minDepth, float maxDepth);

    /// Filters cloud to given density
    PointCloud::Ptr filterDensity(PointCloud::ConstPtr cloud, float leafSize);

    /// Removes the largest planar component from the cloud
    PointCloud::Ptr removePlanar(PointCloud::ConstPtr cloud);

    /// Returns only the largest cluster in the cloud
    PointCloud::Ptr extractLargestCluster(PointCloud::ConstPtr cloud, float cluster_tolerance);

    /// Performs both planar removal and cluster extraction at one
    PointCloud::Ptr segmentCloud(PointCloud::ConstPtr cloud, float cluster_tolerance);

private:
    pcl::PassThrough<pcl::PointXYZRGB> passthrough_filter_;
    pcl::SACSegmentation<pcl::PointXYZRGB> sac_segmentation_;
    pcl::VoxelGrid<pcl::PointXYZRGB> voxelgrid_filter_;
    pcl::EuclideanClusterExtraction<pcl::PointXYZRGB> cluster_extractor_;
    pcl::ExtractIndices<pcl::PointXYZRGB> indice_extractor_;
};

}
