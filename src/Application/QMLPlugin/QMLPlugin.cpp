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
    gazedialog_ = 0;
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
        if (gazedialog_)
        {
            LogInfo("Cannot open new gaze dialog, dialog already open");
            return;
        }
        gazedialog_ = new GazeDialog();
        framework_->Ui()->AddWidgetToScene(gazedialog_, Qt::Dialog);
        gazedialog_->show();
        gazedialog_->move(200, 200);
        connect(gazedialog_, SIGNAL(WindowAccepted(float,int,int,bool,bool,bool)), this, SLOT(GazeParametersAccepted(float,int,int,bool,bool,bool)));
        connect(gazedialog_, SIGNAL(destroyed()), this, SLOT(GazeWindowDestroyed()));
        emit GazeWindowOpened();
    }
}

void QMLPlugin::SetGazeParameters(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse)
{
   gazedialog_->SetValues(center_size, points, rect_size, delta_mode, debug_mode, mouse);
}

void QMLPlugin::GazeParametersAccepted(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse)
{
    emit GazeWindowAccepted(center_size, points, rect_size, delta_mode, debug_mode, mouse);
}

void QMLPlugin::GazeWindowDestroyed()
{
    gazedialog_ = 0;
    delete gazedialog_;
    emit GazeWindowReject();
}

void QMLPlugin::MoveEntity(EC_Placeable *camera, EC_Placeable *entity, float delta_x, float delta_y)
{
    Transform new_transform = entity->gettransform();
    float3 newpos = camera->Orientation() * float3(delta_x, 0, delta_y);
    new_transform.pos += newpos;
    entity->settransform(new_transform);

    //entityPos.position += camera_->GetOrientation() * Vector3df(amountx, -1 * amounty, 0);
    //placeable->settransform(entityPos);
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
