// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"

#include <QList>
#include <QObject>
#include <QImage>

namespace ObjectCapture
{
class KinectCapture;

class CloudProcessor : public QObject
{
    Q_OBJECT

public:
    CloudProcessor();
    ~CloudProcessor();

    PointCloud::Ptr finalCloud() const;

public slots:
    void startCapture();
    void stopCapture();
    void captureCloud();
    void registerClouds();

signals:
    void RGBUpdated(const QImage &frame);
    void registrationFinished();

private slots:

protected:
    void moveToOrigo(PointCloud::Ptr cloud);

private:
    KinectCapture *kinect_capture_;
    QList<PointCloud::ConstPtr> captured_clouds_;
    PointCloud::Ptr final_cloud_;
};

}
