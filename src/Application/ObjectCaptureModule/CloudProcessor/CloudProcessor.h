// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

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

private slots:

private:
    KinectCapture *kinect_capture_;
};

}
