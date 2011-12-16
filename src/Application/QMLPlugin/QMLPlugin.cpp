/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#include "QMLPlugin.h"
#include "EC_QML.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"
#include "CoreDefines.h"
#include "Application.h"
#include "LoggingFunctions.h"

QMLPlugin::QMLPlugin() :
    IModule("QMLPlugin")
{
}

void QMLPlugin::Load()
{  
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_QML>));
}

void QMLPlugin::Uninitialize()
{
}

void QMLPlugin::Unload()
{
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new QMLPlugin();
        fw->RegisterModule(module);
    }
}
