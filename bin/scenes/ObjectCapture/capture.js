engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var objectFinished = false;

if(!server.IsRunning()) {
    ObjectCapture();
}

function ObjectCapture()
{
    print("Starting object capture script");
    var module = framework.GetModuleByName("ObjectCapture");
    module.startCapturing();
    
    var screenEntity = scene.GetEntityByName("screen");
    var component = screenEntity.GetComponent("EC_WidgetCanvas");
    component.SetSubmesh(1);
    component.SetRefreshRate(30);
    
    module.previewFrameUpdated.connect(updateScreen);
    module.objectCaptured.connect(displayObject);

    var or = new Quat(1,0,0,0);
    or.Normalize();
    var po = new float3(0,0,0);
    var scale = new float3(10,10,10);

    module.setLiveCloudPosition(or, po, scale);
    
    var me = scene.GetEntityByName("ObjectCaptureApplication");
    var inputmapper = me.GetOrCreateComponent("EC_InputMapper", 2, false);
    
    inputmapper.contextPriority = 100;
    inputmapper.takeMouseEventsOverQt = false;
    inputmapper.takeKeyboardEventsOverQt = false;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false;
    inputmapper.executionType = 1;

    inputmapper.RegisterMapping("Y", "Capture", 1); // 1 = keypress
    inputmapper.RegisterMapping("U", "Finalize", 1);
    inputmapper.RegisterMapping("I", "startCapturing", 1);
    inputmapper.RegisterMapping("O", "stopCapturing", 1);
    
    me.Action("Capture").Triggered.connect(captureCloud);
    me.Action("Finalize").Triggered.connect(finalizeCloud);
    me.Action("startCapturing").Triggered.connect(startCapturing);
    me.Action("stopCapturing").Triggered.connect(stopCapturing);
}

function updateScreen(image)
{   
    var screenEntity = scene.GetEntityByName("screen");
    var component = screenEntity.GetComponent("EC_WidgetCanvas");
    
    component.Update(image);
}

function startCapturing()
{
    print("Starting capture interface.");
    var module = framework.GetModuleByName("ObjectCapture");
    module.startCapturing();
}

function stopCapturing()
{
    print("Stopping capture interface.");
    var module = framework.GetModuleByName("ObjectCapture");
    module.stopCapturing();
}

function captureCloud()
{
    print("Captured cloud.");
    var module = framework.GetModuleByName("ObjectCapture");
    module.captureCloud();
}

function finalizeCloud()
{
    print("Finalizing cloud.");
    var module = framework.GetModuleByName("ObjectCapture");
    module.finalizeCapturing();
}

function displayObject()
{
    objectFinished = true;
}
