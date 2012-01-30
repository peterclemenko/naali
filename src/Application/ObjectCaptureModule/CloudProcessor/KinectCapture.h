// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"

#include <QObject>
#include <QImage>

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

    bool isRunning();

    void kinect_callback_ (const PointCloud::ConstPtr &cloud);

public slots:
    void startCapture();
    void stopCapture();
    PointCloud::ConstPtr currentCloud() const;

signals:
    void cloudUpdated(PointCloud::ConstPtr cloud);
    void RGBUpdated(const QImage &frame);

private slots:
    void updateRGBImage();

private:
    pcl::Grabber *kinect_interface_;
    PointCloud::ConstPtr current_cloud_;
    QImage rgb_frame_;
};

}
