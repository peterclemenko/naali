#pragma once

#include "IComponent.h"
#include "Scene.h"
#include "SceneInteract.h"


class EC_Portal : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Portal", 41)

public:
    explicit EC_Portal(Scene *scene);
    ~EC_Portal();

    Q_PROPERTY(QString address READ getaddress WRITE setaddress);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, address);

    Q_PROPERTY(QString port READ getport WRITE setport);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, port);

    Q_PROPERTY(QString protocol READ getprotocol WRITE setprotocol);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, protocol);

signals:

public slots:

private slots:
    void UpdateMethod();

    void Update(float);

    void parentClicked(Entity*);

private:
    float3 position_;

    // Sceneinteract
    SceneInteract *pieru_;
};
