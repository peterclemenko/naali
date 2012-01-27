// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"

#include <QObject>

namespace pcl
{
    class Grabber;
}

namespace ObjectCapture
{

class KinectCapture : public QObject
{
    Q_OBJECT

public:
    KinectCapture();
    ~KinectCapture();

    void kinect_callback_ (const PointCloud::ConstPtr &cloud);

public slots:
    void startCapture();
    void stopCapture();

signals:
    void cloudUpdated(PointCloud::ConstPtr cloud);

private slots:

private:
    pcl::Grabber *kinect_interface_;
    PointCloud::ConstPtr current_cloud_;
};

}
