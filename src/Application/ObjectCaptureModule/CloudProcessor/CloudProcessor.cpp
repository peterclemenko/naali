// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"
#include "Math/float3.h"

#include "CloudProcessor.h"
#include "KinectCapture.h"
#include "CloudFilter.h"

#include "pcl/filters/passthrough.h"

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{

CloudProcessor::CloudProcessor() :
    kinect_capture_(new KinectCapture()),
    cloud_filter_(new CloudFilter())
{
    bool check;

    check = connect(kinect_capture_, SIGNAL(RGBUpdated(QImage)), this, SIGNAL(RGBUpdated(QImage)));
    Q_ASSERT(check);
}

CloudProcessor::~CloudProcessor()
{
    SAFE_DELETE(kinect_capture_);
}

void CloudProcessor::startCapture()
{
    if(!kinect_capture_->isRunning())
        kinect_capture_->startCapture();
}

void CloudProcessor::stopCapture()
{
    if(kinect_capture_->isRunning())
        kinect_capture_->stopCapture();
}

void CloudProcessor::captureCloud()
{
    if(kinect_capture_->isRunning() && kinect_capture_->currentCloud().get())
        captured_clouds_.append(kinect_capture_->currentCloud());
}

void CloudProcessor::registerClouds()
{
    if(captured_clouds_.isEmpty())
        return;

    kinect_capture_->stopCapture();

    LogInfo("ObjectCapture: Registering clouds..");

    // fake registration process and only apply passthrough filter for now
    PointCloud::Ptr filtered_cloud(new PointCloud);
    pcl::PassThrough<pcl::PointXYZRGB> passthrough;
    passthrough.setFilterFieldName("z");
    passthrough.setFilterLimits(0.0, 1.4);
    passthrough.setInputCloud(captured_clouds_.at(0));
    passthrough.filter(*filtered_cloud);

    moveToOrigo(filtered_cloud);

    final_cloud_ = filtered_cloud;

    captured_clouds_.clear(); // clear dataset

    LogInfo("ObjectCapture: Registration finished.");
    emit registrationFinished(final_cloud_);
}

void CloudProcessor::moveToOrigo(PointCloud::Ptr cloud)
{
    float3 distance(0, 0, 0);
    for(int i = 0; i < cloud->points.size(); ++i)
    {
        distance.x += cloud->points[i].x;
        distance.y += cloud->points[i].y;
        distance.z += cloud->points[i].z;
    }
    distance /= cloud->points.size();

    for(int i = 0; i < cloud->points.size(); ++i)
    {
        cloud->points[i].x -= distance.x;
        cloud->points[i].y -= distance.y;
        cloud->points[i].z -= distance.z;
    }
}

PointCloud::Ptr CloudProcessor::finalCloud() const
{
    return final_cloud_;
}

}
