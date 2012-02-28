/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#pragma once

#include "FrameworkFwd.h"
#include "IModule.h"
#include "InputAPI.h"
#include "GazeDialog.h"

class QMLPlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor
    QMLPlugin();

    // IModule override
    void Load();

    // IModule override
    void Uninitialize();

    // IModule override
    void Unload();

    // IModule override
    void Initialize();

public slots:
    void HandleKeyPressedEvent(KeyEvent *event);

    // For setting gaze parameters
    void SetGazeParameters(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse);
    void GazeParametersAccepted(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse);
    void GazeWindowRejected();

private:
    InputContextPtr input_;
    GazeDialog *gazedialog_;

signals:
    void GazeWindowOpened();
    void GazeWindowAccepted(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse);
    void GazeWindowReject();
};
