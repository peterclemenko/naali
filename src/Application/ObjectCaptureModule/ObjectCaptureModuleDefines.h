#pragma once

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

namespace ObjectCapture
{
    typedef pcl::PointCloud<pcl::PointXYZRGBA> PointCloud;
    typedef pcl::PointCloud<pcl::Normal> SurfaceNormals;
}
