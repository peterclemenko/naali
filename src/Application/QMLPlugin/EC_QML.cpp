/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_QML.cpp
 *  @brief  EC_QML is a component for showing QML-elements in a texture.
 *  @note   no notes
 */


#include "StableHeaders.h"
#include "EC_QML.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"

#include "InputAPI.h"
#include "MemoryLeakCheck.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "Framework.h"
#include "OgreSceneManager.h"
#include "Renderer.h"
#include <OgreSceneManager.h>
#include "OgreWorld.h"
#include "IComponentFactory.h"

#include <QApplication>
#include <QMouseEvent>
#include <QEvent>

EC_QML::EC_QML(Scene *scene) :
    IComponent(scene),
    renderSubmeshIndex(this, "Render Submesh", 0),
    interactive(this, "Interactive", false),
    qmlsource(this, "QML source", ""),
    renderinterval(this, "Render interval", 40),
    qml_ready(false),
    qmlview_(0)
{
    renderTimer_ = new QTimer();

    // Connect signals from IComponent
    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareQML()), Qt::UniqueConnection);
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeChanged(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(ServerHandleAttributeChange(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
    QObject::connect(renderTimer_, SIGNAL(timeout()), this, SLOT(Render()));

    renderTimer_->setInterval(getrenderinterval());
    if (framework->GetModule<OgreRenderer::OgreRenderingModule>())
        renderer_ = framework->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

    //Create a new input context that EC_QML will use to fetch input.
    input_ = framework->Input()->RegisterInputContext("QMLInput", 100);

    // To be sure that Qt doesn't play tricks on us and miss a mouse release when we're in FPS mode,
    // grab the mouse movement input over Qt.
    input_->SetTakeMouseEventsOverQt(true);

    // Listen on mouse input signals.
    connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), this, SLOT(HandleMouseInputEvent(MouseEvent *)));
}

EC_QML::~EC_QML()
{
    SAFE_DELETE_LATER(qmlview_);
    SAFE_DELETE_LATER(renderTimer_);
}

bool EC_QML::PrepareQML()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return false;

    // Get parent and connect to the signals.
    Entity *parent = ParentEntity();
    assert(parent);
    if (parent)
    {
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
    }
    else
    {
        LogError("PrepareComponent: Could not get parent entity pointer!");
        return false;
    }

    // Get EC_Mesh component.
    mesh_ = GetMeshComponent();
    if (!mesh_)
    {
        // Wait for EC_Mesh to be added.
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        qml_ready = false;
        return false;
    }

    EC_Placeable *placeable = GetPlaceableComponent();
    if (!placeable)
    {
        // Wait for EC_Placeable to be added.
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        qml_ready = false;
        return false;
    }

    // Get EC_WidgetCanvas component
    canvas_ = GetSceneCanvasComponent();
    if (!canvas_)
    {
        //LogError("PrepareComponent: Could not get or create EC_WidgetCanvas component!");
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        qml_ready = false;
        return false;
    }
    else
    {
        if (!qmlview_)
            qmlview_ = new QDeclarativeView();

        qmlview_->setSource(QUrl(getqmlsource()));
        QObject::connect(qmlview_, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(QMLStatus(QDeclarativeView::Status)));

        canvas_->SetSubmesh(getrenderSubmeshIndex());
        canvas_->SetWidget(qmlview_);
        return true;
    }
}

void EC_QML::QMLStatus(QDeclarativeView::Status qmlstatus)
{
    if (framework->IsHeadless())
        return;
    if (qmlstatus == QDeclarativeView::Ready)
    {
        LogInfo("QDeclarativeView has loaded and created the QML component.");

        if (qmlview_->size().width() > 0 && qmlview_->size().height() > 0)
        {
            renderTimer_->start();
            qml_ready = true;
        }
        else
        {
            renderTimer_->stop();
            LogError("Unable to draw the QML component, because it has no size defined!");
            qml_ready = false;
        }
    }

    else if (qmlstatus == QDeclarativeView::Null)
    {
        renderTimer_->stop();
        LogInfo("QDeclarativeView has no source set.");
        qml_ready = false;
    }
    else if (qmlstatus == QDeclarativeView::Loading)
    {
        renderTimer_->stop();
        LogInfo("QDeclarativeView is loading network data.");
        qml_ready = false;
    }
    else if (qmlstatus == QDeclarativeView::Error)
    {
        renderTimer_->stop();
        LogError("One or more errors has occurred.");
        qml_ready = false;
    }
    else
    {
        renderTimer_->stop();
        qml_ready = false;
        LogError("Unknown QDeclarativeView status!");
    }
}

void EC_QML::HandleMouseInputEvent(MouseEvent *mouse)
{
    if(mouse->eventType == MouseEvent::MousePressed && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity == ParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u;
                int ypos = (int)qmlview_->size().height() * result->v;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonPress, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseReleased && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity == ParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u;
                int ypos = (int)qmlview_->size().height() * result->v;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonRelease, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseDoubleClicked && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity == ParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u;
                int ypos = (int)qmlview_->size().height() * result->v;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonDblClick, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseMove && mouse->IsLeftButtonDown())
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity==ParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u;
                int ypos = (int)qmlview_->size().height() * result->v;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseMove, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }
}

void EC_QML::AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType)
{
    if (attribute == &qmlsource)
    {
        qml_ready = false;
        if (PrepareQML())
            qmlview_->setSource(QUrl(getqmlsource()));
    }
    if (attribute == &renderSubmeshIndex)
    {
        canvas_->SetSubmesh(getrenderSubmeshIndex());
    }
    if (attribute == &renderinterval)
    {
        renderTimer_->setInterval(getrenderinterval());
    }
}


void EC_QML::Render()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    /*Scene* scenepointer = ParentEntity()->ParentScene();

    EntityList qmllist = scenepointer->GetEntitiesWithComponent("EC_QML");
    if (qmllist.empty())
        return;*/

    if (qml_ready)
    {
            canvas_->SetSubmesh(getrenderSubmeshIndex());
            canvas_->Update();
    }
}

void EC_QML::ServerHandleAttributeChange(IAttribute *attribute, AttributeChange::Type changeType)
{
    if (GetFramework()->IsHeadless())
        return;

    if (attribute == &qmlsource)
    {
        qml_ready = false;
        if (PrepareQML())
            qmlview_->setSource(QUrl(getqmlsource()));
    }
    if (attribute == &renderSubmeshIndex)
    {
        if (PrepareQML())
            canvas_->SetSubmesh(getrenderSubmeshIndex());
    }
    if (attribute == &renderinterval)
    {
        renderTimer_->setInterval(getrenderinterval());
    }
}


void EC_QML::ComponentAdded(IComponent *component, AttributeChange::Type change)
{
    if(component->TypeName()==EC_Mesh::TypeNameStatic() || component->TypeName()==EC_Placeable::TypeNameStatic() || component->TypeName()==EC_WidgetCanvas::TypeNameStatic())
    {
        PrepareQML();
    }
}

EC_Mesh* EC_QML::GetMeshComponent()
{

    if (ParentEntity())
    {
            IComponent *iComponent =  ParentEntity()->GetComponent("EC_Mesh").get();
            EC_Mesh *mesh = dynamic_cast<EC_Mesh*>(iComponent);
            return mesh;

    }
    else
        LogError("Couldn't get parent entity, returning 0");
    return 0;

}

EC_WidgetCanvas* EC_QML::GetSceneCanvasComponent()
{
    IComponent *iComponent = ParentEntity()->GetComponent("EC_WidgetCanvas").get();
    EC_WidgetCanvas *canvas = dynamic_cast<EC_WidgetCanvas*>(iComponent);

    return canvas;
}

EC_Placeable *EC_QML::GetPlaceableComponent()
{
    if (!ParentEntity())
        return 0;
    IComponent *iComponent = ParentEntity()->GetComponent("EC_Placeable").get();
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(iComponent);
    return placeable;
}

void EC_QML::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    PrepareQML();
}
