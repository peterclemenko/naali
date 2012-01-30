// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"

#include <QList>
#include <QObject>

namespace ObjectCapture
{
class KinectCapture;

class CloudProcessor : public QObject
{
    Q_OBJECT

public:
    CloudProcessor();
    ~CloudProcessor();

public slots:
    void startCapture();
    void stopCapture();
    void captureCloud();

private slots:

private:
    KinectCapture *kinect_capture_;
    QList<PointCloud::ConstPtr> captured_clouds_;
};

}
