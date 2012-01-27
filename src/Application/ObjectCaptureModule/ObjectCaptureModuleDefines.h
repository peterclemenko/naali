#pragma once

#include <pcl-1.4/pcl/point_cloud.h>
#include <pcl-1.4/pcl/point_types.h>

namespace ObjectCapture
{
    typedef pcl::PointCloud<pcl::PointXYZRGB> PointCloud;
    typedef pcl::PointCloud<pcl::Normal> SurfaceNormals;
}
