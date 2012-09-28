// Portal script which enables possibility to make new connection attempts by clicking objects.
// Place this script to your entity which has placeable and visible mesh.
// Jukka Vatjus-Anttila / CIE

print("Portal.js: script initialized in " + me);

var isServer = server.IsRunning();
var firstRun = true;

if (!isServer)
{
    //to track display entity visibility to disable rtt tex update
    var rtt = null; //the current rtt target image used. what happens when it is removed? clean js execption?

    var parentEntity = this.me;
    if (!parentEntity)
        return;

    input.TopLevelInputContext().MouseLeftPressed.connect(mouseLeftPress);
    client.Disconnected.connect(clientDisconnected);


    if (firstRun)
    {
        firstRun = false;
        frame.DelayedExecute(1).Triggered.connect(initialize);
    }

    function initialize()
    {
        print("Initialize!!");
        var camera_ent = scene.GetEntityByName("AvatarCamera");
        var cam = camera_ent.camera;
        cam.StartViewTracking(me);
        cam.EntityEnterView.connect(enterView);
        cam.EntityLeaveView.connect(leaveView);

        var conName = "";
    }

    function mouseLeftPress(event)
    {
        // Get entity from mouseclick location.
        var raycastResult = scene.ogre.Raycast(event.x, event.y, 0xffffffff);
        if(raycastResult.entity != null)
        {
            // Check if clicked entity was parentEntity for this script
            if (raycastResult.entity == parentEntity)
            {
                // This should be changed according to the scene. If no avatar present then this is void.
                var avatar = scene.GetEntityByName("Avatar" + client.GetConnectionID());
                var avatarPos = avatar.placeable.transform.pos;
                var parentPos = parentEntity.placeable.transform.pos;

                // Lets check if controlled avatar is inside arbitary range to initialize login procedure.
                var distance = avatarPos.Distance(parentPos);
                if (distance < 3)
                {

                    // This disconnect should be disabled if multiple simultaneous connections are wanted with multiconnection feature.a
                    //console.ExecuteCommand("Disconnect()");
                    client.Connected.connect(newConnection);
                    client.switchScene.connect(sceneSwitch);
                    switch (me.Name())
                    {
                        // These attributes are hardcoded for portalScene.
                    case "camdisplaywall1":
                        client.Login("localhost", 2346,"lal", "pass", "udp");
                        conName = "127.0.0.1-2346-udp";
                        break;
                    case "camdisplaywall2":
                        client.Login("localhost", 2347,"lal", "pass", "udp");
                        conName = "127.0.0.1-2347-udp";
                        break;
                    case "camdisplaywall3":
                        client.Login("localhost", 2348,"lal", "pass", "udp");
                        conName = "127.0.0.1-2348-udp";
                        break;
                    case "camdisplaywall4":
                        client.Login("localhost", 2349,"lal", "pass", "udp");
                        conName = "127.0.0.1-2349-udp";
                        break;
                    case "camdisplaywall5":
                        client.Login("localhost", 2350,"lal", "pass", "udp");
                        conName = "127.0.0.1-2350-udp";
                        break;
                    case "camdisplaywall6":
                        client.Login("localhost", 2351,"lal", "pass", "udp");
                        conName = "127.0.0.1-2351-udp";
                        break;
                    }
                }
            }
        }
    }
    function clientDisconnected(id)
    {
        if (id == conName)
        {
            me.mesh.SetMaterial(0, "portalMaterial.100.material");
            me.mesh.meshRef = me.mesh.meshRef;
        }

    }

    function sceneSwitch(name)
    {
        client.switchScene.disconnect(sceneSwitch);
        client.Connected.disconnect(newConnection);
    }

    function newConnection(replyData)
    {
        frame.DelayedExecute(1).Triggered.connect(this, init); //XXX dirty hack
    }

    function init()
    {
        var otherScene = framework.Scene().GetScene(conName);
        if (otherScene == null)
            return;
        var cam = otherScene.GetEntityByName("AvatarCamera");
        if (cam)
        {
            cam.GetOrCreateComponent("EC_RttTarget");
            rtt = cam.rtttarget;
            rtt.textureName = "AvatarCamera_" + conName + "_tex";
            print("RTT: " + rtt.textureName);
        }
        else
        {me.mesh.SetMaterial(0, matname);
            cam = otherScene.GetEntityByName("FreeLookCamera");
            cam.GetOrCreateComponent("EC_RttTarget");
            rtt = cam.rtttarget;
            rtt.textureName = "FreeLookCamera_" + conName + "_tex";
        }
        rtt.size_x = 800;
        rtt.size_y = 400;
        rtt.PrepareRtt();
        rtt.SetAutoUpdated(true);
        var matname = rtt.textureName + "_mat"; //XXX add mat name getter to EC_RttTarget
        me.mesh.SetMaterial(0, matname);
        client.switchScene.disconnect(sceneSwitch);
        client.Connected.disconnect(newConnection);
    }
    function enterView(entity)
    {
        if (rtt != null)
        {
            //print("Entity: " + entity.Name() + " has entered the view.");
            rtt.SetAutoUpdate(true);
        }
    }

    function leaveView(entity)
    {
        if (rtt != null)
        {
            //print("Entity: " + entity.Name() + " has left the view.");
            rtt.SetAutoUpdate(false);
        }
    }

}
