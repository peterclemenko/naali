// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ObjectCaptureModule.h"
#include "CloudProcessor/CloudProcessor.h"
#include "MeshReconstructor/MeshReconstructor.h"

#include "Framework.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

namespace ObjectCapture
{
ObjectCaptureModule::ObjectCaptureModule() :
    IModule("ObjectCapture"),
    cloud_processor_(new CloudProcessor()),
    mesh_reconstructor_(0)
{
    bool check;

    check = connect(cloud_processor_, SIGNAL(RGBUpdated(QImage)), this, SIGNAL(previewFrameUpdated(QImage)));
    Q_ASSERT(check);
}

ObjectCaptureModule::~ObjectCaptureModule()
{
    SAFE_DELETE(cloud_processor_);
    SAFE_DELETE(mesh_reconstructor_)
}

void ObjectCaptureModule::Load()
{
    framework_->RegisterDynamicObject("ObjectCapture", this);
}

void ObjectCaptureModule::Initialize()
{
}

void ObjectCaptureModule::Uninitialize()
{
}

void ObjectCaptureModule::Update(f64 frametime)
{
}

void ObjectCaptureModule::startCapturing()
{
    cloud_processor_->startCapture();
}

void ObjectCaptureModule::stopCapturing()
{
    cloud_processor_->stopCapture();
}

void ObjectCaptureModule::captureCloud()
{
    cloud_processor_->captureCloud();
}

}// end of namespace: ObjectCapture

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new ObjectCapture::ObjectCaptureModule();
        fw->RegisterModule(module);
    }
}
