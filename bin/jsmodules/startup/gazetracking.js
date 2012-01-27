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
var center_size = 0.10;
var border_x = 0;
var border_y = 0;
var speed = 0;
var cameraEnt = null;
var maximum_rotate_speed = 2.0;
var maximum_camera_angle_y = 65;
var last_raycast_entity = null;
var action_added = false;
var entity_selected = false;
var selected_entity = null;
var pitch_angle = 0;
var roll_angle = 0;

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
    cameraEnt.Action("GraspGesture").Triggered.connect(GraspGesture);
    cameraEnt.Action("ReleaseGesture").Triggered.connect(ReleaseGesture);
    cameraEnt.Action("PitchAndRoll").Triggered.connect(PitchAndRoll);
    //cameraEnt.Exec(1, "GazeCoordinates", 50, 50);
    //action_added = true;
    //print("Got Camera");
}

function TimerTimeout()
{
    if (!scene)
        return;

    if (!entity_selected)
        HandleCameraRotation();

    else if (entity_selected)
        HandleEntityRotation();
}

function HandleCameraRotation()
{
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
    if (!entity_selected)
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

function GraspGesture()
{
    if (!last_raycast_entity)
        return;

    var placeable = last_raycast_entity.placeable;
    if (placeable.drawDebug == true)
    {
        print("Entity Grasped.");
        entity_selected = true;
        selected_entity = last_raycast_entity;   
    }
}

function ReleaseGesture()
{
    if (entity_selected)
    {
        print("Entity Released.");
        entity_selected = false;
        selected_entity = null;
    }
}

function PitchAndRoll(pitch, roll)
{
    if (!entity_selected)
        return;
    pitch_angle = parseInt(pitch);
    roll_angle = parseInt(roll);    
}

function HandleEntityRotation()
{
    if (!selected_entity)
        return;

    if (pitch_angle >= 30 && pitch_angle < 45)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += 1;
        selected_entity.placeable.transform = transform;   
    }
    else if (pitch_angle >= 45 && pitch_angle < 60)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += 2;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle >= 60 && pitch_angle < 90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += 3;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle <= -30 && pitch_angle > -45)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= 1;
        selected_entity.placeable.transform = transform;   
    }
    else if (pitch_angle <= -45 && pitch_angle > -60)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= 2;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle <= -60 && pitch_angle > -90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= 3;
        selected_entity.placeable.transform = transform;
    }

    if (roll_angle >= 30 && roll_angle < 45)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += 1;
        selected_entity.placeable.transform = transform;   
    }
    else if (roll_angle >= 45 && roll_angle < 60)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += 2;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle >= 60 && roll_angle < 90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += 3;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle <= -30 && roll_angle > -45)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= 1;
        selected_entity.placeable.transform = transform;   
    }
    else if (roll_angle <= -45 && roll_angle > -60)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= 2;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle <= -60 && roll_angle > -90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= 3;
        selected_entity.placeable.transform = transform;
    }
}

