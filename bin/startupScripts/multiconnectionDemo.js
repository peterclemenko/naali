//Date: 30.01.2012
//Made by: Jukka Vatjus-Anttila / Center for Internet Excellence

var doFirst = true;
var scene = null;
var sceneList = null;
var scene1 = null;
var scene2 = null;
var scene3 = null;

// Start this connection process when FrameAPI sends first frameUpdate signal
frame.Updated.connect(StartProcess);
function StartProcess()
{
    // This function starts the whole process. Only run it at 1st frame and then set doFirst to false.
    if (doFirst)
    {
        doFirst = false;
		var isserver = server.IsRunning() || server.IsAboutToStart();
        if (!isserver)
		{
			print("Not server, connecting!");
            frame.DelayedExecute(5).Triggered.connect(this, this.makeConnection);          
		}
    }
}

function makeConnection() {
	print("Making connection to localhost:2345!\n");
	console.ExecuteCommand("connect(localhost,2345,tcp,,)");
    frame.DelayedExecute(5).Triggered.connect(this, this.makeConnection2);          
}
function makeConnection2() {
	print("Making connection to localhost:2346!\n");
	console.ExecuteCommand("connect(localhost,2346,tcp,,)");
    frame.DelayedExecute(5).Triggered.connect(this, this.makeConnection3);          
}
function makeConnection3() {
	print("Making connection to chiru.cie.fi:3457!\n");
	console.ExecuteCommand("connect(chiru.cie.fi,3457,udp,,)");
    frame.DelayedExecute(10).Triggered.connect(this, this.change);          
}

function change() {
	sceneList = client.getSceneNames();
	scene1 = sceneList[0];
	scene2 = sceneList[1];
	scene3 = sceneList[2];
	frame.DelayedExecute(5).Triggered.connect(this, this.swapview3);	
	frame.DelayedExecute(10).Triggered.connect(this, this.swapview1);	
	frame.DelayedExecute(15).Triggered.connect(this, this.swapview2);	
//	print("sceneList: " + lista);
}

function swapview1() {
	print("Swapping to scene: " + scene1);
	scene = framework.Scene().GetScene(scene1);
	var cameraentity = scene.GetEntityByName("AvatarCamera");
	if (cameraentity == null)
		cameraentity = scene.GetEntityByName("FreeLookCamera");
    var camera = cameraentity.camera;
	camera.SetActive(camera);
}
function swapview2() {
	print("Swapping to scene: " + scene2);
	scene = framework.Scene().GetScene(scene2);
	var cameraentity = scene.GetEntityByName("AvatarCamera");
	if (cameraentity == null)
		cameraentity = scene.GetEntityByName("FreeLookCamera");
    var camera = cameraentity.camera;
	camera.SetActive(camera);
}
function swapview3() {
	print("Swapping to scene: " + scene3);
	scene = framework.Scene().GetScene(scene3);
	var cameraentity = scene.GetEntityByName("AvatarCamera");
	if (cameraentity == null)
		cameraentity = scene.GetEntityByName("FreeLookCamera");
    var camera = cameraentity.camera;
	camera.SetActive(camera);
}
