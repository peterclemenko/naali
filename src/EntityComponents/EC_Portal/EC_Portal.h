#pragma once

#include "IComponent.h"
#include "Scene.h"
#include "SceneInteract.h"


class EC_Portal : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Portal", 903)

public:
    explicit EC_Portal(Scene *scene);
    ~EC_Portal();

    Q_PROPERTY(QString address READ getaddress WRITE setaddress)
    DEFINE_QPROPERTY_ATTRIBUTE(QString, address)

    Q_PROPERTY(QString port READ getport WRITE setport)
    DEFINE_QPROPERTY_ATTRIBUTE(QString, port)

    Q_PROPERTY(QString protocol READ getprotocol WRITE setprotocol)
    DEFINE_QPROPERTY_ATTRIBUTE(QString, protocol)

signals:

public slots:

private slots:
    void Update(float);

    void parentClicked(Entity*, Qt::MouseButton);

private:
    /// Position of portal in scene
    float3 position_;

    /// Sceneinteract pointer
    SceneInteract *sceneInteract_;

    /// Parent entity
    Entity *parent_;

    /// Parent scene
    Scene *scene_;
};
