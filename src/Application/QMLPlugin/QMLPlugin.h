/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#pragma once

#include "FrameworkFwd.h"
#include "IModule.h"

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

};
