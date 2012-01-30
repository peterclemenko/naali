// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CloudProcessor.h"
#include "KinectCapture.h"

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{

CloudProcessor::CloudProcessor() :
    kinect_capture_(new KinectCapture())
{

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
    if(kinect_capture_->isRunning())
        captured_clouds_.append(kinect_capture_->currentCloud());
}

}
