// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"

#include "KinectCapture.h"

#include <pcl-1.4/pcl/io/openni_grabber.h>
#include <QRgb>

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{

KinectCapture::KinectCapture() :
    rgb_update_frequency_(15)
{
    kinect_interface_ = new pcl::OpenNIGrabber();
    rgb_frame_ = QImage(640, 480, QImage::Format_ARGB32);

    boost::function<void (const PointCloud::ConstPtr&)> f = boost::bind(&KinectCapture::kinect_callback_, this, _1);
    kinect_interface_->registerCallback(f);

    bool check;
    check = connect(&rgb_update_timer_, SIGNAL(timeout()), this, SLOT(updateRGBImage()));

    rgb_update_timer_.start(1000/rgb_update_frequency_);
}

KinectCapture::~KinectCapture()
{
    kinect_interface_->stop();
    delete kinect_interface_;
}

void KinectCapture::startCapture()
{
    if(kinect_interface_) {
        kinect_interface_->start();
        LogDebug("ObjectCaptureModule: Starting Kinect interface..");
    }
}

void KinectCapture::stopCapture()
{
    if(kinect_interface_) {
        kinect_interface_->stop();
        LogDebug("ObjectCaptureModule: Stopping Kinect interface");
    }
}

bool KinectCapture::isRunning()
{
    if(!kinect_interface_)
        return false;
    else
        return kinect_interface_->isRunning();
}

PointCloud::ConstPtr KinectCapture::currentCloud()
{
    cloud_mutex_.lock();
    PointCloud::ConstPtr cloud;
    cloud = current_cloud_;
    cloud_mutex_.unlock();
    return cloud;
}

void KinectCapture::updateRGBImage()
{
    /// \todo need to create new image for each frame?
    /// \todo figure out a way avoid innecessary copying of rgb values.
    if(!kinect_interface_->isRunning())
        return;

    if(cloud_mutex_.tryLock()) {
        if(!current_cloud_.get())
            return;

        PointCloud::ConstPtr cloud = current_cloud_;
        cloud_mutex_.unlock();

        cloud->points.size(); // Just here to ensure compiler doesn't optimize cloud out

        QRgb value;
        for(int u = 0; u < 480; u++) {
            for(int v = 0; v < 640; v++) {
                if(isnan(cloud->points[(u*640)+v].x))
                    value = qRgb(255, 63, 139);
                else
                    value = qRgb(cloud->points[(u*640)+v].r, cloud->points[(u*640)+v].g, cloud->points[(u*640)+v].b);
                rgb_frame_.setPixel(v, u, value);
            }
        }

        emit RGBUpdated(rgb_frame_);
    }
}

void KinectCapture::kinect_callback_(const PointCloud::ConstPtr &cloud)
{
    if(cloud_mutex_.tryLock()) {
        current_cloud_.reset();
        current_cloud_ = cloud;
        cloud_mutex_.unlock();

        emit cloudUpdated(current_cloud_);
    }
}

}
