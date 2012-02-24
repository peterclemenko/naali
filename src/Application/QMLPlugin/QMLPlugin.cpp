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
#include "UiProxyWidget.h"
#include "UiAPI.h"

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

void QMLPlugin::Initialize()
{
    //Create a new input context that QMLPlugin will use to fetch input.
    input_ = framework_->Input()->RegisterInputContext("QMLPluginInput", 101);
    connect(input_.get(), SIGNAL(KeyPressed(KeyEvent*)), this, SLOT(HandleKeyPressedEvent(KeyEvent*)));

}

void QMLPlugin::HandleKeyPressedEvent(KeyEvent *event)
{
    if (event->Sequence() == QKeySequence(Qt::ShiftModifier + Qt::Key_G))
    {
        gazedialog_ = new GazeDialog();
        framework_->Ui()->AddWidgetToScene(gazedialog_);
        gazedialog_->show();
        connect(gazedialog_, SIGNAL(WindowAccepted(float,int,int,bool,bool)), this, SLOT(GazeParametersAccepted(float,int,int,bool,bool)));
        emit GazeWindowOpened();
    }
}

void QMLPlugin::SetGazeParameters(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode)
{
   gazedialog_->SetValues(center_size, points, rect_size, delta_mode, debug_mode);
}

void QMLPlugin::GazeParametersAccepted(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode)
{
    emit GazeWindowAccepted(center_size, points, rect_size, delta_mode, debug_mode);
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
