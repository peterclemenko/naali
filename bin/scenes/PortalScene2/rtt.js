var rtt = null; //the current rtt target image used. what happens when it is removed? clean js execption?

/*//to track display entity visibility to disable rtt tex update
var freecam_ent = scene.GetEntityByName("FreeLookCamera");
var cam = freecam_ent.camera;
cam.StartViewTracking(me);
//cam.EntityEnterView.connect(displayEnterFreecam);
//cam.EntityLeaveView.connect(displayLeaveFreecam);

client.Connected.connect(newConnection)

function newConnection()
{
    print("RTT: New connection!");
    frame.DelayedExecute(1).Triggered.connect(this, init); //XXX dirty hack

}

function init()
{
    var otherScene = framework.Scene().GetScene("312a");
    var cam = otherScene.GetEntityByName("AvatarCamera");
    cam.GetOrCreateComponent("EC_RttTarget");
    rtt = cam.rtttarget;
    rtt.textureName = "AvatarCamera" + "_tex";
    rtt.size_x = 300;
    rtt.size_y = 200;
    rtt.PrepareRtt();
    rtt.SetAutoUpdated(true);
    var matname = rtt.textureName + "_mat"; //XXX add mat name getter to EC_RttTarget
    me.mesh.SetMaterial(2, matname);
}*/
