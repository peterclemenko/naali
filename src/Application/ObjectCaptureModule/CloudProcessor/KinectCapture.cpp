// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "KinectCapture.h"

#include <pcl-1.4/pcl/io/openni_grabber.h>

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{

KinectCapture::KinectCapture()
{
    kinect_interface_ = new pcl::OpenNIGrabber();

    boost::function<void (const PointCloud::ConstPtr&)> f = boost::bind(&KinectCapture::kinect_callback_, this, _1);
    kinect_interface_->registerCallback(f);
}

KinectCapture::~KinectCapture()
{

}

void KinectCapture::startCapture()
{
    if(kinect_interface_)
        kinect_interface_->start();
}

void KinectCapture::stopCapture()
{
    if(kinect_interface_)
        kinect_interface_->stop();
}

void KinectCapture::kinect_callback_(const PointCloud::ConstPtr &cloud)
{
    current_cloud_.reset();
    current_cloud_ = cloud;
    emit cloudUpdated(current_cloud_);
}

}
