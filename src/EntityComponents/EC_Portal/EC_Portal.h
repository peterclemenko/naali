#pragma once

#include "IComponent.h"
#include "Scene.h"
#include "SceneInteract.h"

/// EntityComponent that initiates new connection to specified address.
/**
<table class="header">
<tr>
<td>
<h2>Portal</h2>
EntityComponent that initiates new connection to specified address.
NOTE: Assumes the entity already has: EC_Placeable. Otherwise EC_Portal cannot get location properly. Connection initiates only if avatar/freelookcam is near the entity.

<b>Attributes</b>:
<ul>
<li>QString: address
<div>Specifies destination server address.</div>
<li>QString: port
<div>Specifies destination server port.</div>
<li>QString: protocol
<div>Specifies destination server protocol.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on ...</b>
</table>
*/


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
