//Currently, this script needs QMLPlugin to work correctly.

if (framework.IsHeadless())
    return;

framework.Scene().SceneAdded.connect(OnSceneAdded);
ui.MainWindow().WindowResizeEvent.connect(MainWindowResized);


if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
}

//Adjusts the dead zone for camera rotation in the center of the screen.
//0.5 or higher makes the dead zone full screen, so camera won't rotate from gaze coordinates.
var center_size = 0.25;

//Adjust the maximum rotation speed of the camera.
var maximum_rotate_speed = 1.5;

//Adjusts the maximum angle for the camera turning up and down, so you can't go upside down.
var maximum_camera_angle_y = 65;

//Adjusts the entity rotating speed for the thresholds.
var rotate_speed_1 = 1;
var rotate_speed_2 = 7;
var rotate_speed_3 = 25;

//If this is true, entity will rotated directly to the angle of the sensor.
//Set it to false for continuous toggled rotation.
var delta_mode = true;

//Adjusts the thresholds for the sensor to toggle rotation.
var rotate_threshold_1 = 30;
var rotate_threshold_2 = 45;
var rotate_threshold_3 = 60;

//Defines the amount of gaze coordinates from which the average is counted from.
var amount_of_points = 30;

//The size of the FrustumQuery rect for selecting objects.
var rect_size = 25;

//If true, shows a second rectangle for the current gaze coordinates, in addition to average coordinates.
var debug_mode = false;

var scene = null;
var timer = new QTimer();
var gaze_x = 0;
var gaze_y = 0;
var delta_center_x = 0;
var delta_center_y = 0;
var screen_width = 0;
var screen_height = 0;
var border_x = 0;
var border_y = 0;
var speed = 0;
var last_raycast_entity = null;
var action_added = false;
var entity_selected = false;
var selected_entity = null;
var pitch_angle = 0;
var roll_angle = 0;
var movement_mode = false;
var delta_roll = 0;
var delta_pitch = 0;
var gaze_counter = 0;
var gaze_average_x = 0;
var gaze_average_y = 0;
var gaze_sum_x = 0;
var gaze_sum_y = 0;
var gaze_points_x = [];
var gaze_points_y = [];
var index = 0;
var movement_speed = 0.1;
var qmlmodule = 0;


if (!framework.IsHeadless())
{
        var label = new QLabel();
        label.objectName = "GazeAverageLabel";
        label.setStyleSheet("QLabel#GazeAverageLabel { padding: 0px; background-color: rgba(230,230,230,0); border: 2px solid red; font-size: 16px; }");
        label.text = "";
        label.resize(2*rect_size, 2*rect_size);

        var proxy = new UiProxyWidget(label);
        ui.AddProxyWidgetToScene(proxy);
        proxy.x = 50
        proxy.y = 50;
        proxy.windowFlags = 0;
        proxy.visible = true;

        var label2 = new QLabel();
        label2.objectName = "GazeCurrentLabel";
        label2.setStyleSheet("QLabel#GazeCurrentLabel { padding: 0px; background-color: rgba(230,230,230,100); border: 0px solid black; font-size: 16px; }");
        label2.text = "";
        label2.resize(20, 20);

        var proxy2 = new UiProxyWidget(label2);
        ui.AddProxyWidgetToScene(proxy2);
        proxy2.x = 50
        proxy2.y = 50;
        proxy2.windowFlags = 0;
        proxy2.visible = false;

        qmlmodule = framework.GetModuleByName("QMLPlugin");
        qmlmodule.GazeWindowOpened.connect(SendGazeParameters);
        qmlmodule.GazeWindowAccepted.connect(SetGazeParameters);
}

function SendGazeParameters()
{
    qmlmodule.SetGazeParameters(center_size, amount_of_points, rect_size, delta_mode, debug_mode);
}

function SetGazeParameters(c_size, points, r_size, del_mode, deb_mode)
{
    center_size = parseFloat(c_size);
    amount_of_points = parseInt(points);
    rect_size = parseInt(r_size);
    delta_mode = del_mode;
    debug_mode = deb_mode;
    label.resize(2*rect_size, 2*rect_size);
    proxy2.visible = debug_mode;
} 

timer.timeout.connect(TimerTimeout);

function OnSceneAdded(scenename)
{
    if (framework.IsHeadless())
        return;

    screen_width = ui.GraphicsView().viewport().size.width();
    screen_height = ui.GraphicsView().viewport().size.height();
    scene = framework.Scene().GetScene(scenename);
    timer.start(25);
    action_added = false;
    gaze_x = parseInt(screen_width / 2);
    gaze_y = parseInt(screen_height / 2);

    /*var inputContext = input.RegisterInputContextRaw("GazeTrackingInput", 102);
    inputContext.SetTakeMouseEventsOverQt(true);
    inputContext.MouseMove.connect(HandleMouseMove);*/

    border_x = parseInt(center_size * screen_width);
    border_y = parseInt(center_size * screen_height);
}

function MainWindowResized(width, height)
{
    screen_width = width;
    screen_height = height;
    
    border_x = parseInt(center_size * screen_width);
    border_y = parseInt(center_size * screen_height);
}

function AddActionToFreeLookCamera()
{
    var cameraEnt = scene.GetEntityByName("FreeLookCamera");
    if (!cameraEnt)
        return;
    cameraEnt.Action("GazeCoordinates").Triggered.connect(GazeCoordinates);
    cameraEnt.Action("GraspGesture").Triggered.connect(GraspGesture);
    cameraEnt.Action("ReleaseGesture").Triggered.connect(ReleaseGesture);
    cameraEnt.Action("PitchAndRoll").Triggered.connect(PitchAndRoll);
    cameraEnt.Action("SwitchGesture").Triggered.connect(SwitchGesture);
    action_added = true;
    print("Actions added to FreeLookCamera");
}

function TimerTimeout()
{
    if (!scene)
        return;

    if (!action_added)
        AddActionToFreeLookCamera();


    if (entity_selected && !movement_mode)
        HandleEntityRotation();
    if (entity_selected && movement_mode)
        HandleEntityMovement();

    if (!entity_selected)
    {
        HandleCameraRotation();
        HandleCameraMovement();
    }

}

function SwitchGesture()
{
    if (!entity_selected)
        return;
    movement_mode = !movement_mode;
}

function HandleCameraRotation()
{
    if (parseInt(delta_center_x) > border_x)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
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
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
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
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
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
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
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

function HandleCameraMovement()
{
    if (entity_selected)
        return;
    //print(pitch_angle + ", " + roll_angle);
    if (pitch_angle >= rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Move", "back");
    }
    else if (pitch_angle <= -rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Move", "forward");
    }
    else
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Stop", "forward");
        cameraEnt.Exec(1, "Stop", "back");
    }

    if (roll_angle >= rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Move", "right");
    }
    else if (roll_angle <= -rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Move", "left");
    }
    else
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Stop", "left");
        cameraEnt.Exec(1, "Stop", "right");
    }
}

function HandleEntityMovement()
{
    if (!selected_entity)
        return;

    if (pitch_angle >= rotate_threshold_1)
    {
        var transform = selected_entity.placeable.transform;
        transform.pos.z += movement_speed;
        selected_entity.placeable.transform = transform;
    }

    else if (pitch_angle <= -rotate_threshold_1)
    {
        var transform = selected_entity.placeable.transform;
        transform.pos.z -= movement_speed;
        selected_entity.placeable.transform = transform;
    }

    if (roll_angle >= rotate_threshold_1)
    {
        var transform = selected_entity.placeable.transform;
        transform.pos.x += movement_speed;
        selected_entity.placeable.transform = transform;
    }

    else if (roll_angle <= -rotate_threshold_1)
    {
        var transform = selected_entity.placeable.transform;
        transform.pos.x -= movement_speed;
        selected_entity.placeable.transform = transform;
    }

}
function HandleMouseMove(mouse)
{
    if (!scene)
        return;
    
    if (gaze_counter < amount_of_points)
    {
        gaze_points_x.unshift(parseInt(mouse.x));
        gaze_points_y.unshift(parseInt(mouse.y));
        gaze_counter += 1;
    }
    else
    {
        gaze_points_x.unshift(parseInt(mouse.x));
        gaze_points_y.unshift(parseInt(mouse.y));
        gaze_points_x.pop();
        gaze_points_y.pop();

        for (index = 0; index < amount_of_points; index++)
        {
            gaze_sum_x += gaze_points_x[index];
            gaze_sum_y += gaze_points_y[index];
        }

        gaze_average_x = gaze_sum_x / amount_of_points;
        gaze_average_y = gaze_sum_y / amount_of_points;
        gaze_sum_x = 0;
        gaze_sum_y = 0;

        gaze_x = gaze_average_x;
        gaze_y = gaze_average_y;
    }

    delta_center_x = (screen_width / 2) - gaze_points_x[0];
    delta_center_y = (screen_height / 2) - gaze_points_y[0];

    if (!entity_selected)
        EntitySelection();

    proxy.x = gaze_x - rect_size;

    proxy.y = gaze_y - rect_size;

    proxy2.x = gaze_points_x[0];
    proxy2.y = gaze_points_y[0];
}

function GazeCoordinates(x, y)
{
    if (!scene)
        return;

    if (gaze_counter < amount_of_points)
    {
        gaze_points_x.unshift(parseInt(x));
        gaze_points_y.unshift(parseInt(y));
        gaze_counter += 1;
    }
    else
    {
        gaze_points_x.unshift(parseInt(x));
        gaze_points_y.unshift(parseInt(y));
        gaze_points_x.pop();
        gaze_points_y.pop();

        for (index = 0; index < amount_of_points; index++)
        {
            gaze_sum_x += gaze_points_x[index];
            gaze_sum_y += gaze_points_y[index];
        }

        gaze_average_x = gaze_sum_x / amount_of_points;
        gaze_average_y = gaze_sum_y / amount_of_points;
        gaze_sum_x = 0;
        gaze_sum_y = 0;

        /*if (((gaze_points_x[0] - gaze_average_x) > (screen_width / 2)) || ((gaze_points_x[0] - gaze_average_x) < (-screen_width / 2)))
        {
            gaze_average_x = gaze_points_x[0];
            for (index = 1; index < amount_of_points; index++)
            {
                gaze_points_x[index] = gaze_points_x[0];
            }

        }
        if (((gaze_points_y[0] - gaze_average_y) > (screen_width / 2)) || ((gaze_points_y[0] - gaze_average_y) < (-screen_height / 2)))
        {
            gaze_average_y = gaze_points_y[0];
            for (index = 1; index < amount_of_points; index++)
            {
                gaze_points_y[index] = gaze_points_y[0];
            }
        }*/

        gaze_x = gaze_average_x;
        gaze_y = gaze_average_y;
    }

    delta_center_x = (screen_width / 2) - gaze_points_x[0];
    delta_center_y = (screen_height / 2) - gaze_points_y[0];

    if (!entity_selected)
        EntitySelection();

    if (gaze_x > screen_width)
        proxy.x = screen_width - 100;
    else if (gaze_x < 0)
        proxy.x = 0;
    else
        proxy.x = gaze_x - rect_size;

    if (gaze_y > screen_height)
        proxy.y = screen_height - 100;
    else if (gaze_y < 0)
        proxy_y = 0;
    else
        proxy.y = gaze_y - rect_size;

    proxy2.x = gaze_x - 10;
    proxy2.y = gaze_y - 10;
}

function EntitySelection()
{

    var cameraEnt = scene.GetEntityByName("FreeLookCamera");
    if (!cameraEnt)
        return;
    var camera_position = cameraEnt.placeable.transform.pos;

    var closest_entity = scene.ogre.FrustumQuery(gaze_x - rect_size, gaze_y - rect_size, gaze_x + rect_size, gaze_y + rect_size, camera_position);

    if (closest_entity)
    {
        if (closest_entity.GetComponent("EC_Placeable"))
        {
            if (closest_entity != last_raycast_entity && last_raycast_entity)
            {
                var placeable = last_raycast_entity.placeable;
                placeable.drawDebug = false;
                last_raycast_entity.placeable = placeable;
            }

            last_raycast_entity = closest_entity;
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

        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "Stop", "left");
        cameraEnt.Exec(1, "Stop", "right");
        cameraEnt.Exec(1, "Stop", "forward");
        cameraEnt.Exec(1, "Stop", "back");
    }
}

function ReleaseGesture()
{
    if (entity_selected)
    {
        print("Entity Released.");
        entity_selected = false;
        selected_entity = null;
        movement_mode = false;
    }
}

function PitchAndRoll(pitch, roll)
{
    delta_pitch = parseInt(pitch - pitch_angle);
    delta_roll = parseInt(roll - roll_angle);

    pitch_angle = parseInt(pitch);
    roll_angle = parseInt(roll);
}

function HandleEntityRotation()
{
    if (!selected_entity)
        return;

    if (delta_mode)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += delta_pitch;
        transform.rot.y += delta_roll;
        selected_entity.placeable.transform = transform;
        return;
    }

    if (pitch_angle >= rotate_threshold_1 && pitch_angle < rotate_threshold_2)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += rotate_speed_1;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle >= rotate_threshold_2 && pitch_angle < rotate_threshold_3)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += rotate_speed_2;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle >= rotate_threshold_3 && pitch_angle < 90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x += rotate_speed_3;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle <= -rotate_threshold_1 && pitch_angle > -rotate_threshold_2)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= rotate_speed_1;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle <= -rotate_threshold_2 && pitch_angle > -rotate_threshold_3)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= rotate_speed_2;
        selected_entity.placeable.transform = transform;
    }
    else if (pitch_angle <= -rotate_threshold_3 && pitch_angle > -90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.x -= rotate_speed_3;
        selected_entity.placeable.transform = transform;
    }

    if (roll_angle >= rotate_threshold_1 && roll_angle < rotate_threshold_2)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += rotate_speed_1;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle >= rotate_threshold_2 && roll_angle < rotate_threshold_3)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += rotate_speed_2;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle >= rotate_threshold_3 && roll_angle < 90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y += rotate_speed_3;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle <= -rotate_threshold_1 && roll_angle > -rotate_threshold_2)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= rotate_speed_1;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle <= -rotate_threshold_2 && roll_angle > -rotate_threshold_3)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= rotate_speed_2;
        selected_entity.placeable.transform = transform;
    }
    else if (roll_angle <= -rotate_threshold_3 && roll_angle > -90)
    {
        var transform = selected_entity.placeable.transform;
        transform.rot.y -= rotate_speed_3;
        selected_entity.placeable.transform = transform;
    }
}
