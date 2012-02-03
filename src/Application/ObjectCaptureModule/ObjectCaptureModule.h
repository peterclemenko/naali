/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ObjectCaptureModule.h
 *  @brief  Chiru 3D object capture and reconstruction module.
 */

#pragma once

#include "IModule.h"
#include "CoreStringUtils.h"

#include "ObjectCaptureModuleDefines.h"

#include <QObject>
#include <QImage>

namespace ObjectCapture
{
class CloudProcessor;
class MeshReconstructor;

/**
 *  XMPP Communications support for tundra
 *
 *  Uses QXMPP library to provide presence, text and voice communications.
 *
 *  See usage example in <insert sample scene dir here>
 *
 *  Work in progress
 */
class ObjectCaptureModule : public IModule
{
    Q_OBJECT

public:
    /// Default constructor.
    ObjectCaptureModule();

    /// Destructor.
    ~ObjectCaptureModule();

    /// IModule override.
    void Load();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);


public slots:
    /// Starts the Kinect capturing interface
    void startCapturing();

    /// Stops the Kinect capturing interface without processing the clouds
    void stopCapturing();

    /// Saves current cloud
    void captureCloud();

    /// Stops capturing and processes the clouds to a mesh
    void finalizeCapturing();

signals:
    void previewFrameUpdated(const QImage &frame);
    void objectCaptured(unsigned int entity_id);

private slots:
    void registrationFinished();
    void cloudProcessingFinished();

private:
    CloudProcessor *cloud_processor_;
    MeshReconstructor *mesh_reconstructor_;
};

} // end of namespace: ObjectCapture
