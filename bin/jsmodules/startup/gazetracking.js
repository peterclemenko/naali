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

//Adjusts the entity movement speed when moving it.
var movement_speed = 0.1;

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

//If true, uses mouse input as gaze coordinates.
var use_mouse = false;

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
var gaze_window_open = false;
var can_throw = false;
var use_statusbutton = true;
var statusbutton = null;
var last_mouse_x = 0;
var last_mouse_y = 0;
var delta_mouse_x = 0;
var delta_mouse_y = 0;
var camera_speed = 0;
var sweep_x_recognized = false;
var sweep_y_recognized = false;
var camera_moving = false;

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
    label2.setStyleSheet("QLabel#GazeCurrentLabel { padding: 0px; background-color: rgba(255,0,0,100); border: 0px solid black; font-size: 16px; }");
    label2.text = "";
    label2.resize(20, 20);

    var proxy2 = new UiProxyWidget(label2);
    ui.AddProxyWidgetToScene(proxy2);
    proxy2.x = 50
    proxy2.y = 50;
    proxy2.windowFlags = 0;
    proxy2.visible = false;

    if (framework.GetModuleByName("QMLPlugin"))
    {
        qmlmodule = framework.GetModuleByName("QMLPlugin");
        qmlmodule.GazeWindowOpened.connect(SendGazeParameters);
        qmlmodule.GazeWindowAccepted.connect(SetGazeParameters);
        qmlmodule.GazeWindowReject.connect(GazeWindowRejected);
    }
    else
    {
        print("QMLPlugin not in use, cannot use gaze properties dialog.");
    }

    var inputContext = input.RegisterInputContextRaw("GazeTrackingInput", 102);

    // Connect gestures
    if (inputContext.GestureStarted && inputContext.GestureUpdated)
    {
	    inputContext.GestureStarted.connect(GestureStarted);
	    inputContext.GestureUpdated.connect(GestureUpdated);
    }
    
}

function SendGazeParameters()
{
    qmlmodule.SetGazeParameters(center_size, amount_of_points, rect_size, delta_mode, debug_mode, use_mouse);
    gaze_window_open = true;
}

function GazeWindowRejected()
{
    gaze_window_open = false;
}

function SetGazeParameters(c_size, points, r_size, del_mode, deb_mode, mouse)
{
    center_size = parseFloat(c_size);
    amount_of_points = parseInt(points);
    rect_size = parseInt(r_size);

    delta_mode = del_mode;
    debug_mode = deb_mode;
    use_mouse = mouse;

    label.resize(2*rect_size, 2*rect_size);
    proxy2.visible = debug_mode;

    if (use_mouse)
    {
        inputContext.SetTakeMouseEventsOverQt(true);
        inputContext.MouseMove.connect(HandleMouseMove);
        inputContext.MouseEventReceived.connect(HandleMouseEvent);
        amount_of_points = 1;
    }
    else
    {
        inputContext.SetTakeMouseEventsOverQt(false);
        inputContext.MouseMove.disconnect(HandleMouseMove);
        inputContext.MouseEventReceived.disconnect(HandleMouseEvent);
        amount_of_points = 30;
    }
} 

timer.timeout.connect(Update);

function OnSceneAdded(scenename)
{
    if (framework.IsHeadless())
        return;

    if (scenename == "TundraServer")
        return;

    screen_width = ui.GraphicsView().viewport().size.width();
    screen_height = ui.GraphicsView().viewport().size.height();
    scene = framework.Scene().GetScene(scenename);
    timer.start(25);
    action_added = false;
    gaze_x = parseInt(screen_width / 2);
    gaze_y = parseInt(screen_height / 2);

    border_x = parseInt(center_size * screen_width);
    border_y = parseInt(center_size * screen_height);

    if (use_statusbutton) 
    {
	    print("adding statusbutton");
	    engine.IncludeFile("lib/overlaybutton.js");
	
	    statusbutton = MakeOverlayButton(220, 20);
	    statusbutton.text = "No object selected";
    }
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
    cameraEnt.Action("ThrowGesture").Triggered.connect(ThrowGesture);
    action_added = true;
    print("Actions added to FreeLookCamera");
}

function Update()
{
    if (!scene)
        return;

    if (!action_added)
        AddActionToFreeLookCamera();

    if (gaze_window_open)
        return;

    if (entity_selected && !movement_mode)
    {
        HandleEntityRotation();
        LookAtSelectedEntity();
    }
    if (entity_selected && movement_mode)
    {
        HandleEntityMovement();
        HandleCameraRotation();
    }

    if (!entity_selected)
    {
        HandleCameraRotation();
        //HandleCameraMovement();
        EntitySelection();
    }

}

function SwitchGesture()
{
    if (!entity_selected)
        return;

    if(movement_mode)
    {
        movement_mode = !movement_mode;
        label.setStyleSheet("QLabel#GazeAverageLabel { padding: 0px; background-color: rgba(230,230,230,0); border: 2px solid green; font-size: 16px; }");    
    }
    else
    {
        movement_mode = true;
        label.setStyleSheet("QLabel#GazeAverageLabel { padding: 0px; background-color: rgba(230,230,230,0); border: 2px solid purple; font-size: 16px; }");
    }
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

    //if (parseInt(delta_center_y) > border_y)
    //{
    //    var cameraEnt = scene.GetEntityByName("FreeLookCamera");
    //    if (!cameraEnt)
    //        return;

    //    speed = (delta_center_y - border_y) / 100;

    //    if (speed > maximum_rotate_speed)
    //        speed = maximum_rotate_speed;
    //    var transform = cameraEnt.placeable.transform;
    //    transform.rot.x += speed;
    //    if (transform.rot.x > maximum_camera_angle_y)
    //        transform.rot.x = maximum_camera_angle_y;
    //    cameraEnt.placeable.transform = transform;

    //}

    //else if (parseInt(delta_center_y) < -border_y)
    //{
    //    var cameraEnt = scene.GetEntityByName("FreeLookCamera");
    //    if (!cameraEnt)
    //        return;

    //    speed = (delta_center_y + border_y) / 100;

    //    if (speed < -maximum_rotate_speed)
    //        speed = -maximum_rotate_speed;
    //    var transform = cameraEnt.placeable.transform;
    //    transform.rot.x += speed;
    //    if (transform.rot.x < -maximum_camera_angle_y)
    //        transform.rot.x = -maximum_camera_angle_y;
    //    cameraEnt.placeable.transform = transform;
    //}
}

function HandleCameraMovement()
{
    if (entity_selected)
        return;

    if (use_mouse)
    {
        if (sweep_y_recognized)
        {
            var cameraEnt = scene.GetEntityByName("FreeLookCamera");
            if (!cameraEnt)
                return;
            if (camera_speed < 0)
            {
                cameraEnt.Exec(1, "MoveWithSpeed", "forward", -0.5);
            }
            else if (camera_speed > 0)
            {
                cameraEnt.Exec(1, "MoveWithSpeed", "forward", 0.5);
            }
            sweep_y_recognized = false;
        }
   
        if (sweep_x_recognized)
        {
            var cameraEnt = scene.GetEntityByName("FreeLookCamera");
            if (!cameraEnt)
                return;
            if (camera_speed < 0)
            {
                cameraEnt.Exec(1, "MoveWithSpeed", "left", -0.5);
            }
            else if (camera_speed > 0)
            {
                cameraEnt.Exec(1, "MoveWithSpeed", "right", 0.5);
            }
            sweep_x_recognized = false;
        }
        return;
    }

    fwspeed = Math.pow((pitch_angle/100),7)
    lrspeed = Math.pow((roll_angle/100),7)

    if (pitch_angle >= rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "MoveWithSpeed", "back", fwspeed);
    }
    else if (pitch_angle <= -rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "MoveWithSpeed", "forward", fwspeed);
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
        cameraEnt.Exec(1, "MoveWithSpeed", "right", lrspeed);
    }
    else if (roll_angle <= -rotate_threshold_1)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "MoveWithSpeed", "left", lrspeed);
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

    if (use_mouse)
    {
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        if ((Math.abs(parseInt(delta_mouse_x)) > 15) || (Math.abs(parseInt(delta_mouse_y)) > 15))
            return;
        qmlmodule.MoveEntity(cameraEnt.placeable, selected_entity.placeable, delta_mouse_x / 20, delta_mouse_y / 20);
        delta_mouse_y = 0;
        delta_mouse_x = 0;
        return;
    }

    if (pitch_angle >= rotate_threshold_1)
    {
	var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        movement_speed = Math.pow((pitch_angle/100),7)
        qmlmodule.MoveEntity(cameraEnt.placeable, selected_entity.placeable, 0, parseFloat(movement_speed));

    }

    else if (pitch_angle <= -rotate_threshold_1)
    {
	var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        movement_speed = Math.pow((pitch_angle/100),7)
        qmlmodule.MoveEntity(cameraEnt.placeable, selected_entity.placeable, 0, parseFloat(movement_speed));
    }

    if (roll_angle >= rotate_threshold_1)
    {
	var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        movement_speed = Math.pow((roll_angle/100),7)
        qmlmodule.MoveEntity(cameraEnt.placeable, selected_entity.placeable, parseFloat(movement_speed), 0);
    }

    else if (roll_angle <= -rotate_threshold_1)
    {
	var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        movement_speed = Math.pow((roll_angle/100),7)
        qmlmodule.MoveEntity(cameraEnt.placeable, selected_entity.placeable, parseFloat(movement_speed), 0);
    }

}
function HandleMouseMove(mouse)
{
    if (!scene)
        return;

    if (gaze_window_open)
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

    proxy.x = gaze_x - rect_size;

    proxy.y = gaze_y - rect_size;

    proxy2.x = gaze_points_x[0] - 10;
    proxy2.y = gaze_points_y[0] - 10;
    delta_mouse_x = mouse.x - last_mouse_x;
    delta_mouse_y = mouse.y - last_mouse_y;
    last_mouse_x = mouse.x;
    last_mouse_y = mouse.y;
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

        gaze_x = gaze_average_x;
        gaze_y = gaze_average_y;
    }

    delta_center_x = (screen_width / 2) - gaze_points_x[0];
    delta_center_y = (screen_height / 2) - gaze_points_y[0];

    if (gaze_x > screen_width)
        proxy.x = screen_width - rect_size;
    else if (gaze_x < 0)
        proxy.x = 0;
    else
        proxy.x = gaze_x - rect_size;

    if (gaze_y > screen_height)
        proxy.y = screen_height - rect_size;
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
            if (use_statusbutton)
		        statusbutton.text = "Gazing at: " + last_raycast_entity.Name();
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

    if (last_raycast_entity.Name() == "Torus")
        return;

    if(entity_selected)
        return;

    var placeable = last_raycast_entity.placeable;
    if (placeable.drawDebug == true)
    {
        print("Entity Grasped.");
        entity_selected = true;
       
        label.setStyleSheet("QLabel#GazeAverageLabel { padding: 0px; background-color: rgba(230,230,230,0); border: 2px solid green; font-size: 16px; }");
        selected_entity = last_raycast_entity;

	    if (use_statusbutton)
	        statusbutton.text = "Grasped: " + last_raycast_entity.Name();

        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
        cameraEnt.Exec(1, "StopAll");
    }
}

function ReleaseGesture()
{
    if (entity_selected)
    {
        print("Entity Released.");
        entity_selected = false;
        label.setStyleSheet("QLabel#GazeAverageLabel { padding: 0px; background-color: rgba(230,230,230,0); border: 2px solid red; font-size: 16px; }");
        if (use_statusbutton)
	            statusbutton.text = "No object selected";
        selected_entity = null;
        movement_mode = false;
    }
}

function ThrowGesture()
{
    if (selected_entity && selected_entity.Name() == "Fish")
    {
        scene.GetEntityByName("Throwing").Exec(1, "ThrowObject", "Fish", "asdf");
	if (use_statusbutton) {
	    statusbutton.text = "Throwing entity: " + selected_entity.Name();
	    frame.DelayedExecute(2.0).Triggered.connect(function(t) { statusbutton.text = ""; });
	}
        print("Throwing fish...");
    }
    else 
    {
        print("Fish not selected.");
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
        if (use_mouse)
        {
            if ((Math.abs(parseInt(delta_mouse_x)) > 15) || (Math.abs(parseInt(delta_mouse_y)) > 15))
                return;
            transform.rot.x -= delta_mouse_y;
            transform.rot.y += delta_mouse_x;
            delta_mouse_y = 0;
            delta_mouse_x = 0;
        }
        else
        {
            transform.rot.x += delta_pitch;
            transform.rot.y += delta_roll;    
        }
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

function GestureStarted(gestureEvent)
{
    if (!use_mouse)
        return;
    if (gestureEvent.GestureType() == Qt.TapAndHoldGesture)
    {
        if (!entity_selected)
            GraspGesture();
        else
            ReleaseGesture();
        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PanGesture)
    {
        var offset = gestureEvent.Gesture().offset.toPoint();
        //HandleMouseLookX(offset.x());
        //HandleMouseLookY(offset.y());
        gestureEvent.Accept();
    }

}

function GestureUpdated(gestureEvent)
{
    print(gestureEvent.GestureType());
    if (!use_mouse)
        return;
    if (gestureEvent.GestureType() == Qt.PanGesture)
    {
        
        var delta = gestureEvent.Gesture().delta.toPoint();
        delta_roll = delta.x();
        delta_pitch = delta.y();
        print("Pan gesture: " + delta_roll + ", " + delta_pitch);
        //HandleMouseLookX(delta.x());
        //HandleMouseLookY(delta.y());
        gestureEvent.Accept();
    }
}

function HandleMouseEvent(mouseEvent)
{
    if (mouseEvent.GetEventType() == 3) //Pressed
    {
        delta_mouse_x = 0;
        delta_mouse_y = 0;
    }
    if (mouseEvent.GetEventType() == 4) //Released
    {
        if (delta_mouse_x > 10)
        {
            sweep_x_recognized = true;
            camera_speed = 1;
            delta_mouse_x = 0;
            delta_mouse_y = 0;    
            return;
        }
        else if (delta_mouse_x < -10)
        {
            sweep_x_recognized = true;
            camera_speed = -1;
            delta_mouse_x = 0;
            delta_mouse_y = 0;
            return;
        }
        
        if (delta_mouse_y > 10)
        {
            sweep_y_recognized = true;
            camera_speed = 1;
            delta_mouse_x = 0;
            delta_mouse_y = 0;
            return;
        }
        else if (delta_mouse_y < -10)
        {
            sweep_y_recognized = true;
            camera_speed = -1;
            delta_mouse_x = 0;
            delta_mouse_y = 0;
            return;
        }
        var cameraEnt = scene.GetEntityByName("FreeLookCamera");
        if (!cameraEnt)
            return;
 
        cameraEnt.Exec(1, "StopAll");

        
        
    }
    else if (mouseEvent.GetEventType() == 5) //Doubleclick
    {
         SwitchGesture();
    }
}


function LookAtSelectedEntity()
{
    var cameraEnt = scene.GetEntityByName("FreeLookCamera");
    if (!cameraEnt)
        return;
    
    var transform = selected_entity.placeable.transform;
    var camera = cameraEnt.camera;
    //camera.lookAt(transform.pos);    
}
