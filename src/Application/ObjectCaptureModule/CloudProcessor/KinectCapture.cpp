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

KinectCapture::KinectCapture()
{
    kinect_interface_ = new pcl::OpenNIGrabber();
    rgb_frame_ = QImage(640, 480, QImage::Format_RGB888);

    boost::function<void (const PointCloud::ConstPtr&)> f = boost::bind(&KinectCapture::kinect_callback_, this, _1);
    kinect_interface_->registerCallback(f);
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

PointCloud::ConstPtr KinectCapture::currentCloud() const
{
    if(current_cloud_.get())
        return current_cloud_;
}

void KinectCapture::updateRGBImage()
{
    /// \todo need to create new image for each frame?
    /// \todo figure out a way avoid innecessary copying of rgb values.
    QRgb value;

    for(int u = 0; u < 480; u++) {
        for(int v = 0; v < 640; v++) {
            if(isnan(current_cloud_->points[(u*640)+v].x))
                value = qRgb(255, 63, 139);
            else
                value = qRgb(current_cloud_->points[(u*640)+v].r, current_cloud_->points[(u*640)+v].g, current_cloud_->points[(u*640)+v].b);
            rgb_frame_.setPixel(v, u, value);
        }
    }

    emit RGBUpdated(rgb_frame_);
}

void KinectCapture::kinect_callback_(const PointCloud::ConstPtr &cloud)
{
    current_cloud_.reset();
    current_cloud_ = cloud;

    updateRGBImage();

    emit cloudUpdated(current_cloud_);
}

}
