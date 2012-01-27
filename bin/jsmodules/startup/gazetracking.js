framework.Scene().SceneAdded.connect(OnSceneAdded);
ui.MainWindow().WindowResizeEvent.connect(MainWindowResized);


if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
}

var scene = null;
var timer = new QTimer();
var gaze_x = 0;
var gaze_y = 0;
var delta_center_x = 0;
var delta_center_y = 0;
var screen_width = 0;
var screen_height = 0;
var center_size = 0.05;
var border_x = 0;
var border_y = 0;
var speed = 0;
var cameraEnt = 0;
var maximum_rotate_speed = 2.0;
var maximum_camera_angle_y = 65;
var last_raycast_entity = null;
var action_added = false;

timer.timeout.connect(TimerTimeout);

function OnSceneAdded(scenename)
{  
    screen_width = ui.GraphicsView().viewport().size.width();
    screen_height = ui.GraphicsView().viewport().size.height();
    scene = framework.Scene().GetScene(scenename);
    timer.start(25);
    //print("Scene added");
    AddActionToFreeLookCamera();
        
    //var inputContext = input.RegisterInputContextRaw("GazeTrackingInput", 102);
    //inputContext.SetTakeMouseEventsOverQt(true);
    //inputContext.MouseMove.connect(HandleMouseMove);
}

function MainWindowResized(width, height)
{
    screen_width = width;
    screen_height = height;
}

function AddActionToFreeLookCamera()
{
    cameraEnt = scene.GetEntityByName("FreeLookCamera");
    if (!cameraEnt)
        return;
    cameraEnt.Action("GazeCoordinates").Triggered.connect(GazeCoordinates);
    //cameraEnt.Exec(1, "GazeCoordinates", 50, 50);
    //action_added = true;
    //print("Got Camera");
}

function TimerTimeout()
{
    if (!scene)
        return;

    if (parseInt(delta_center_x) > border_x)
    {
        cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;

        speed = (delta_center_x - border_x) / 100;
        
        if (speed > maximum_rotate_speed)
            speed = maximum_rotate_speed;
        var transform = cameraEnt.placeable.transform;
        transform.rot.y += speed;
        cameraEnt.placeable.transform = transform;
    }
    
    else if (parseInt(delta_center_x) < -border_x)
    {
        cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;

        speed = (delta_center_x + border_x) / 100;
        
        if (speed < -maximum_rotate_speed)
            speed = -maximum_rotate_speed;
        var transform = cameraEnt.placeable.transform;
        transform.rot.y += speed;
        cameraEnt.placeable.transform = transform;
    }

    if (parseInt(delta_center_y) > border_y)
    {
        cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;

        speed = (delta_center_y - border_y) / 100;
        
        if (speed > maximum_rotate_speed)
            speed = maximum_rotate_speed;
        var transform = cameraEnt.placeable.transform;
        transform.rot.x += speed;
        if (transform.rot.x > maximum_camera_angle_y)
            transform.rot.x = maximum_camera_angle_y;            
        cameraEnt.placeable.transform = transform;
        
    }

    else if (parseInt(delta_center_y) < -border_y)
    {
        cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;

        speed = (delta_center_y + border_y) / 100;
        
        if (speed < -maximum_rotate_speed)
            speed = -maximum_rotate_speed;
        var transform = cameraEnt.placeable.transform;
        transform.rot.x += speed;
        if (transform.rot.x < -maximum_camera_angle_y)
            transform.rot.x = -maximum_camera_angle_y;
        cameraEnt.placeable.transform = transform;
        

        
    }
}

function HandleMouseMove(mouse)
{
    /*if (!scene)
        return;
    border_x = parseInt(center_size * screen_width);
    border_y = parseInt(center_size * screen_height);
    gaze_x = mouse.x;
    gaze_y = mouse.y;
    delta_center_x = (screen_width / 2) - gaze_x;
    delta_center_y = (screen_height / 2) - gaze_y;
    EntitySelection();*/
}

function GazeCoordinates(x, y)
{
    print("GazeCoordinates: " + x + ", " + y);
    if (!scene)
        return;
    border_x = parseInt(center_size * screen_width);
    border_y = parseInt(center_size * screen_height);
    gaze_x = x;
    gaze_y = y;
    delta_center_x = (screen_width / 2) - gaze_x;
    delta_center_y = (screen_height / 2) - gaze_y;
    EntitySelection();
}

function EntitySelection()
{
    var raycastResult = renderer.Raycast(gaze_x, gaze_y);
    if (raycastResult.entity)
    {
        if (raycastResult.entity.GetComponent("EC_Placeable"))
        {
            if (raycastResult.entity != last_raycast_entity && last_raycast_entity)
            {
                var placeable = last_raycast_entity.placeable;
                placeable.drawDebug = false;
                last_raycast_entity.placeable = placeable;
            }

            last_raycast_entity = scene.GetEntity(raycastResult.entity.id);
            var placeable = last_raycast_entity.placeable;
            placeable.drawDebug = true;
            last_raycast_entity.placeable = placeable;
        }
        
    }
    else
    {
        if (last_raycast_entity)
        {
            var placeable = last_raycast_entity.placeable;
            placeable.drawDebug = false;
            last_raycast_entity.placeable = placeable;
        }
    }


}

