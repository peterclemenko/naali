var showUi = true;
if (server.IsRunning() && framework.IsHeadless())
    showUi = false;

if (showUi)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.objectName = "InfoLabel";
    label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230,175); border: 1px solid black; font-size: 16px; }");
    label.text = "This scene implements an \"flying avatar application\".\n\nMouse right: Look around\nw,a,s,d: Move\nc: descent\nspace: ascent";
    var proxy = ui.AddWidgetToScene(label);

    // Check if the browser ui is present (or anything else on top left corner)
    proxy.x = 20;
    if (ui.GraphicsView().GetVisibleItemAtCoords(20,40) == null)
        proxy.y = 40;
    else
        proxy.y = 100;
    proxy.windowFlags = 0;
    proxy.visible = true;
    proxy.focusPolicy = Qt.NoFocus;
}
