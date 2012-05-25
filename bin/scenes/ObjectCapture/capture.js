engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

if(!server.IsRunning()) {
    ObjectCapture();
}

function ObjectCapture()
{
    print("\nStarting object capture script");
    var module = framework.GetModuleByName("ObjectCapture");
    module.startCapturing();

    module.setLiveCloudPosition(Quat(1,0,0,0), float3(0,0,-30), float3(10,10,10));
    module.setGlobalModelPosition(Quat(1,0,0,0), float3(20,0,-30), float3(10,10,10));
    module.setFinalMeshPosition(Quat(1,0,0,0), float3(-20,0,-30), float3(10,10,10));
    
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


    var dialog = new QDialog();
    dialog.setStyleSheet("QDialog {background-color: transparent;}");
    var layout = new QHBoxLayout(dialog);

    var buttonCapture = new QPushButton("Capture");
    var buttonFinalize = new QPushButton("Finalize");
    var buttonStart = new QPushButton("Start Capturing");
    var buttonStop = new QPushButton("Stop Capturing");

    layout.addWidget(buttonCapture, 0, Qt.AlignLeft);
    layout.addWidget(buttonFinalize, 0, Qt.AlignLeft);
    layout.addWidget(buttonStart, 0, Qt.AlignLeft);
    layout.addWidget(buttonStop, 0, Qt.AlignLeft);

    var proxy = new UiProxyWidget(dialog);

    ui.AddProxyWidgetToScene(proxy);
    proxy.x = 100;
    proxy.y = 100;
    proxy.windowFlags = 0;
    proxy.visible = true;

    buttonCapture.clicked.connect(captureCloud);
    buttonFinalize.clicked.connect(finalizeCloud);
    buttonStart.clicked.connect(startCapturing);
    buttonStop.clicked.connect(stopCapturing);
    
    me.Action("Capture").Triggered.connect(captureCloud);
    me.Action("Finalize").Triggered.connect(finalizeCloud);
    me.Action("startCapturing").Triggered.connect(startCapturing);
    me.Action("stopCapturing").Triggered.connect(stopCapturing);
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
