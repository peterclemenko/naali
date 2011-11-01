// !ref: default_avatar.avatar

// A simple walking avatar running on client side

engine.IncludeFile("av_common.js");

if (!server.IsRunning() && !framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
}

// A simple walking avatar with physics & 1st/3rd person camera
function SimpleAvatar(entity, comp)
{
    // Store the entity reference
    this.me = entity;

    this.rotateSpeed = 100.0;
    this.mouseRotateSensitivity = 0.2;
    this.moveForce = 15.0;
    this.flySpeedFactor = 0.25;
    this.dampingForce = 3.0;
    this.walkAnimSpeed = 0.5;
    this.avatarCameraHeight = 1.0;
    this.avatarMass = 10;

    // Tracking motion with entity actions
    this.motionX = 0;
    this.motionY = 0;
    this.motionZ = 0;

    // Clientside yaw, pitch & rotation state
    this.yaw = 0;
    this.pitch = 0;
    this.rotate = 0;

    // Needed bools for logic
    this.isServer = server.IsRunning() || server.IsAboutToStart();
    this.ownAvatar = false;
    this.flying = false;
    this.falling = false;
    this.isMouseLookLockedOnX = true;

    // Animation detection
    this.standAnimName = "Stand";
    this.walkAnimName = "Walk";
    this.flyAnimName = "Fly";
    this.hoverAnimName = "Hover";
    this.animList = [this.standAnimName, this.walkAnimName, this.flyAnimName, this.hoverAnimName];

    this.animsDetected = false;
    this.listenGesture = false;

    if (this.isServer)
        this.ServerInitialize();
    else
        this.ClientInitialize();
}

SimpleAvatar.prototype.OnScriptObjectDestroyed = function() {
    // Must remember to manually disconnect subsystem signals, otherwise they'll continue to get signalled
    if (this.isServer)
    {
        frame.Updated.disconnect(this, this.AnimationUpdate);
        scene.physics.Updated.disconnect(this, this.ClientUpdatePhysics);
    }
    else
        frame.Updated.disconnect(this, this.ClientUpdate);
}

SimpleAvatar.prototype.ServerInitialize = function() {
    var rigidbody = this.me.GetOrCreateComponentRaw("EC_RigidBody");
    rigidbody.AssertAuthority(false);
}

SimpleAvatar.prototype.ClientInitialize2 = function() {
    // This is ServerInitialize in the standard avatar app.

    // Create the avatar component & set the avatar appearance. The
    // avatar component will create the mesh & animationcontroller,
    // once the avatar asset has loaded

    log("ClientInitialize2 called");

    var avatar = this.me.GetOrCreateComponent("EC_Avatar");

    var me_connid = dc_get(me, "connectionID");
    var thisclient_connid = client.GetConnectionID();

    log("checking if own avatar, client has " + thisclient_connid + ", me ent has " + me_connid);

    if (me_connid != thisclient_connid) {
	log(".. nope.");
	return;
    }
    // Try to dig login param avatar. This seemed like the easiest way to do it.
    // Parse the connection id from the entity name and get a connection for him,
    // check if a custom av url was passed when this client logged in.

    // var entName = this.me.name;
    // var indexNum = entName.substring(6); // 'Avatar' == 6, we want the number after that
    // var clientPtr = server.GetUserConnection(parseInt(indexNum, 10));
    
    // Default avatar ref
    var avatarurl = "default_avatar.avatar";
    
    var avatarurlProp = client.GetLoginProperty("avatarurl");
    if (avatarurlProp && avatarurlProp.length > 0) {
	debug.Log("Avatar from login parameters enabled: " + avatarurlProp);
	avatarurl = avatarurlProp;
    }
    
    var r = avatar.appearanceRef;
    r.ref = avatarurl;
    avatar.appearanceRef = r;

    // Get rigid body component and set physics properties
    var rigidbody = this.me.GetOrCreateComponent("EC_RigidBody");
    rigidbody.AssertAuthority(true);
    var sizeVec = new float3(0.5, 2.4, 0.5);
    rigidbody.mass = this.avatarMass;
    rigidbody.shapeType = 3; // Capsule
    rigidbody.size = sizeVec;
    rigidbody.angularFactor = float3.zero; // Set zero angular factor so that body stays upright

    // Create dynamic component attributes for disabling/enabling functionality, and for camera distance / 1st/3rd mode
    var attrs = this.me.dynamiccomponent;
    attrs.CreateAttribute("bool", "enableWalk");
    attrs.CreateAttribute("bool", "enableJump");
    attrs.CreateAttribute("bool", "enableFly");
    attrs.CreateAttribute("bool", "enableRotate");
    attrs.CreateAttribute("bool", "enableAnimation");
    attrs.CreateAttribute("bool", "enableZoom");
    attrs.CreateAttribute("real", "cameraDistance");
    attrs.SetAttribute("enableWalk", true);
    attrs.SetAttribute("enableJump", true);
    attrs.SetAttribute("enableFly", true);
    attrs.SetAttribute("enableRotate", true);
    attrs.SetAttribute("enableAnimation", true);
    attrs.SetAttribute("enableZoom", true);
    attrs.SetAttribute("cameraDistance", 7.0);

    // Create an inactive proximitytrigger, so that other proximitytriggers can detect the avatar
    // var proxtrigger = me.GetOrCreateComponent("EC_ProximityTrigger");
    // proxtrigger.active = false;

    // Hook to physics update
    scene.physics.Updated.connect(this, this.ClientUpdatePhysics);

    // Hook to tick update for animation update
    frame.Updated.connect(this, this.AnimationUpdate);

    // Connect actions. These come from the client side inputmapper
    this.me.Action("Move").Triggered.connect(this, this.ClientHandleMove);
    this.me.Action("Stop").Triggered.connect(this, this.ClientHandleStop);
    this.me.Action("ToggleFly").Triggered.connect(this, this.ClientHandleToggleFly);
    this.me.Action("SetRotation").Triggered.connect(this, this.ClientHandleSetRotation);

    rigidbody.PhysicsCollision.connect(this, this.ClientHandleCollision);
}

SimpleAvatar.prototype.AnimationUpdate = function(frametime) {
    var attrs = this.me.dynamiccomponent;

    if (!this.animsDetected) {
        this.CommonFindAnimations();
    }

    // If walk enable was toggled off, make sure the motion state is cleared
    if (!attrs.GetAttribute("enableWalk"))
    {
        this.motionX = 0;
        this.motionZ = 0;
    }
    
    // If flying enable was toggled off, but we are still flying, disable now
    if ((this.flying) && (!attrs.GetAttribute("enableFly")))
        this.ClientSetFlying(false);

    this.CommonUpdateAnimation(frametime);
}

SimpleAvatar.prototype.ServerHandleCollision = function(ent, pos, normal, distance, impulse, newCollision) {
    if (this.falling && newCollision) {
        this.falling = false;
        this.SetAnimationState();
    }
}

SimpleAvatar.prototype.ClientUpdatePhysics = function(frametime) {
    var placeable = this.me.placeable;
    var rigidbody = this.me.rigidbody;
    var attrs = this.me.dynamiccomponent;

    // don't fall to china
    if (transform.pos.z < -50) {
	transform.pos.z = 20;
	placeable.transform = transform;
	ServerHandleToggleFly();
    }

    if (!this.flying) {
        // Apply motion force
        // If diagonal motion, normalize
        if (this.motionX != 0 || this.motionZ != 0) {
            var impulse = new float3(this.motionX, 0, -this.motionZ).Normalized().Mul(this.moveForce);
            var tm = placeable.LocalToWorld();
            impulse = tm.MulDir(impulse);
            rigidbody.ApplyImpulse(impulse);
        }

        // Apply jump
        if (this.motionY == 1 && !this.falling) {
            if (attrs.GetAttribute("enableJump")) {
                var jumpVec = new float3(0, 75, 0);
                this.motionY = 0;
                this.falling = true;
                rigidbody.ApplyImpulse(jumpVec);
            }
        }

        // Apply damping. Only do this if the body is active, because otherwise applying forces
        // to a resting object wakes it up
        if (rigidbody.IsActive()) {
            var dampingVec = rigidbody.GetLinearVelocity();
            dampingVec.x = -this.dampingForce * dampingVec.x;
            dampingVec.y = 0;
            dampingVec.z = -this.dampingForce * dampingVec.z;
            rigidbody.ApplyImpulse(dampingVec);
        }
    } else {
        // Manually move the avatar placeable when flying
        // this has the downside of no collisions.
        // Feel free to reimplement properly with mass enabled.
        var avTransform = placeable.transform;

        // Make a vector where we have moved
        var moveVec = new float3(this.motionX * this.flySpeedFactor, this.motionY * this.flySpeedFactor, -this.motionZ * this.flySpeedFactor);

        // Apply that with av looking direction to the current position
        var offsetVec = placeable.LocalToWorld().MulDir(moveVec);
        avTransform.pos.x = avTransform.pos.x + offsetVec.x;
        avTransform.pos.y = avTransform.pos.y + offsetVec.y;
        avTransform.pos.z = avTransform.pos.z + offsetVec.z;

        placeable.transform = avTransform;
    }
}

SimpleAvatar.prototype.ClientHandleMove = function(param) {
    log("ClientHandleMove " + param);
    if (dc_get(this.me, "enableWalk")) {
        if (param == "forward") {
            this.motionZ = 1;
        }
        if (param == "back") {
            this.motionZ = -1;
        }
        if (param == "right") {
            this.motionX = 1;
        }
        if (param == "left") {
            this.motionX = -1;
        }
    }

    if (param == "up") {
        this.motionY = 1;
    }
    if (param == "down") {
        this.motionY = -1;
    }

    this.SetAnimationState();
}

SimpleAvatar.prototype.ClientHandleStop = function(param) {
    log("ClientHandleStop " + param);
    if ((param == "forward") && (this.motionZ == 1)) {
        this.motionZ = 0;
    }
    if ((param == "back") && (this.motionZ == -1)) {
        this.motionZ = 0;
    }
    if ((param == "right") && (this.motionX == 1)) {
        this.motionX = 0;
    }
    if ((param == "left") && (this.motionX == -1)) {
        this.motionX = 0;
    }
    if ((param == "up") && (this.motionY == 1)) {
        this.motionY = 0;
    }
    if ((param == "down") && (this.motionY == -1)) {
        this.motionY = 0;
    }

    this.SetAnimationState();
}

SimpleAvatar.prototype.ClientHandleToggleFly = function() {
    this.ClientSetFlying(!this.flying);
}

SimpleAvatar.prototype.ClientSetFlying = function(newFlying) {
    var attrs = this.me.dynamiccomponent;
    if (!attrs.GetAttribute("enableFly"))
        newFlying = false;

    if (this.flying == newFlying)
        return;

    var rigidbody = this.me.rigidbody;
    this.flying = newFlying;
    if (this.flying) {
        rigidbody.mass = 0;
    } else {
        var placeable = this.me.placeable;
        // Set mass back for collisions
        rigidbody.mass = this.avatarMass;
        // Push avatar a bit to the fly direction
        // so the motion does not just stop to a wall
        var moveVec = new float3(this.motionX * 120, this.motionY * 120, -this.motionZ * 120);
        var pushVec = placeable.LocalToWorld().MulDir(moveVec);
        rigidbody.ApplyImpulse(pushVec);
    }
    this.SetAnimationState();
}

SimpleAvatar.prototype.ClientHandleSetRotation = function(param) {
    if (dc_get(this.me, "enableRotate")) {
        var rot = new float3(0, parseFloat(param), 0);
        this.me.rigidbody.SetRotation(rot);
    }
}

SimpleAvatar.prototype.SetAnimationState = function() {
    // Not flying: Stand, Walk or Crouch
    var animName = this.standAnimName;
    if ((this.motionX != 0) || (this.motionZ != 0)) {
        animName = this.walkAnimName;
    }

    // Flying: Fly if moving forward or back, otherwise hover
    if (this.flying || this.falling) {
        animName = this.flyAnimName;
        if (this.motionZ == 0)
            animName = this.hoverAnimName;
    }

    if (animName == "") {
        return;
    }

    // Update the variable to sync to client if changed
    var animcontroller = this.me.animationcontroller;
    if (animcontroller != null) {
        if (animcontroller.animationState != animName) {
            animcontroller.animationState = animName;
        }
    }
}

SimpleAvatar.prototype.ClientInitialize = function() {
    // Set all avatar entities as temprary also on the clients.
    // This is already done in the server but the info seems to not travel to the clients!
    this.me.SetTemporary(true);
    
    // TODO t1 ver connected Move action to ClientHandleMove here unconditionally, check

    // Check if this is our own avatar
    // Note: bad security. For now there's no checking who is allowed to invoke actions
    // on an entity, and we could theoretically control anyone's avatar

    var me_connid = dc_get(me, "connectionID");
    var thisclient_connid = client.GetConnectionID();

    log("checking if own avatar, client has " + thisclient_connid + ", me ent has " + me_connid);

    if (me_connid == thisclient_connid) {
        this.ownAvatar = true;
        this.ClientCreateInputMapper();
        this.ClientCreateAvatarCamera();
        var soundlistener = this.me.GetOrCreateComponent("EC_SoundListener", 2, false);
        soundlistener.active = true;

        this.me.Action("MouseScroll").Triggered.connect(this, this.ClientHandleMouseScroll);
        this.me.Action("Zoom").Triggered.connect(this, this.ClientHandleKeyboardZoom);
        this.me.Action("Rotate").Triggered.connect(this, this.ClientHandleRotate);
        this.me.Action("StopRotate").Triggered.connect(this, this.ClientHandleStopRotate);
	this.me.ClientInitialize2();
    }
    else
    {
        // Make hovering name tag for other clients
        var clientName = this.me.GetComponent("EC_Name");
        if (clientName != null) {
            // Description holds the actual login name
            if (clientName.description != "") {
                var nameTag = this.me.GetOrCreateComponent("EC_HoveringText", 2, false);
                if (nameTag != null) {
                    nameTag.SetTemporary(true);
                    nameTag.text = clientName.description;
                    var pos = nameTag.position;
                    pos.y = 1.3;
                    nameTag.position = pos;
                    nameTag.fontSize = 90;
                    var color = new Color(0.2, 0.2, 0.2, 1.0);
                    nameTag.backgroundColor = color;
                    var font_color = new Color(1.0, 1.0, 1.0, 1.0);                
                    nameTag.fontColor = font_color;
                }
            }
        }
    }

    // Hook to tick update to update visual effects (both own and others' avatars)
    frame.Updated.connect(this, this.ClientUpdate);
}

SimpleAvatar.prototype.IsCameraActive = function() {
    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return false;
    var camera = cameraentity.camera;
    return camera.IsActive();
}

SimpleAvatar.prototype.ClientUpdate = function(frametime) {
    // Tie enabled state of inputmapper to the enabled state of avatar camera
    if (this.ownAvatar) {
        var avatarcameraentity = scene.GetEntityByName("AvatarCamera");
        var inputmapper = this.me.inputmapper;
        if ((avatarcameraentity != null) && (inputmapper != null)) {
            var active = avatarcameraentity.camera.IsActive();
            if (inputmapper.enabled != active) {
                inputmapper.enabled = active;
            }
        }
        this.ClientUpdateRotation(frametime);
        this.ClientUpdateAvatarCamera(frametime);
    }

    if (!this.animsDetected) {
        this.CommonFindAnimations();
    }
    this.CommonUpdateAnimation(frametime);
}

SimpleAvatar.prototype.ClientCreateInputMapper = function() {
    // Create a nonsynced inputmapper
    var inputmapper = this.me.GetOrCreateComponent("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 101;
    inputmapper.takeMouseEventsOverQt = false;
    inputmapper.takeKeyboardEventsOverQt = false;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false; // Disable repeat keyevent sending over network, not needed and will flood network
    inputmapper.executionType = 1; // Execute actions on client

    // Key pressed -actions
    inputmapper.RegisterMapping("W", "Move(forward)", 1); // 1 = keypress
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right))", 1);
    inputmapper.RegisterMapping("Up", "Move(forward)", 1);
    inputmapper.RegisterMapping("Down", "Move(back)", 1);
    inputmapper.RegisterMapping("F", "ToggleFly()", 1);
    inputmapper.RegisterMapping("Space", "Move(up)", 1);
    inputmapper.RegisterMapping("C", "Move(down)", 1);

    // Key released -actions
    inputmapper.RegisterMapping("W", "Stop(forward)", 3); // 3 = keyrelease
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Up", "Stop(forward)", 3);
    inputmapper.RegisterMapping("Down", "Stop(back)", 3);
    inputmapper.RegisterMapping("Space", "Stop(up)", 3);
    inputmapper.RegisterMapping("C", "Stop(down)", 3);

    // Connect mouse gestures
    var inputContext = inputmapper.GetInputContext();
    inputContext.GestureStarted.connect(this, this.GestureStarted);
    inputContext.GestureUpdated.connect(this, this.GestureUpdated);
    inputContext.MouseMove.connect(this, this.ClientHandleMouseMove);

    // Local mapper for mouse scroll and rotate
    var inputmapper = this.me.GetOrCreateComponent("EC_InputMapper", "CameraMapper", 2, false);
    inputmapper.contextPriority = 100;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.executionType = 1; // Execute actions locally
    inputmapper.RegisterMapping("+", "Zoom(in)", 1);
    inputmapper.RegisterMapping("-", "Zoom(out)", 1);
    inputmapper.RegisterMapping("Left", "Rotate(left)", 1);
    inputmapper.RegisterMapping("Right", "Rotate(right))", 1);
    inputmapper.RegisterMapping("Left", "StopRotate(left)", 3);
    inputmapper.RegisterMapping("Right", "StopRotate(right))", 3);
}

SimpleAvatar.prototype.ClientCreateAvatarCamera = function() {
    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
    {
        cameraentity = scene.CreateLocalEntity();
        cameraentity.SetName("AvatarCamera");
        cameraentity.SetTemporary(true);
    }

    var camera = cameraentity.GetOrCreateComponent("EC_Camera");
    var placeable = cameraentity.GetOrCreateComponent("EC_Placeable");

    camera.SetActive();
    
    var parentRef = placeable.parentRef;
    parentRef.ref = this.me; // Parent camera to avatar, always
    placeable.parentRef = parentRef;

    // Set initial position
    this.ClientUpdateAvatarCamera();
}

SimpleAvatar.prototype.GestureStarted = function(gestureEvent) {
    if (!this.IsCameraActive())
        return;
    if (gestureEvent.GestureType() == Qt.PanGesture)
    {
        this.listenGesture = true;

        var attrs = this.me.dynamiccomponent;
        if (attrs.GetAttribute("enableRotate")) {
            var x = new Number(gestureEvent.Gesture().offset.toPoint().x());
            this.yaw += x;
            this.me.Exec(2, "SetRotation", this.yaw.toString());
        }

        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PinchGesture)
        gestureEvent.Accept();
}

SimpleAvatar.prototype.GestureUpdated = function(gestureEvent) {
    if (!IsCameraActive())
        return;

    if (gestureEvent.GestureType() == Qt.PanGesture && this.listenGesture == true)
    {
        // Rotate avatar with X pan gesture
        delta = gestureEvent.Gesture().delta.toPoint();
        this.yaw += delta.x;
        this.me.Exec(2, "SetRotation", this.yaw.toString());

        // Start walking or stop if total Y len of pan gesture is 100
        var walking = false;
        if (this.me.animationcontroller.animationState == this.walkAnimName)
            walking = true;
        var totalOffset = gestureEvent.Gesture().offset.toPoint();
        if (totalOffset.y() < -100)
        {
            if (walking) {
                this.me.Exec(2, "Stop", "forward");
                this.me.Exec(2, "Stop", "back");
            } else
                this.me.Exec(2, "Move", "forward");
            listenGesture = false;
        }
        else if (totalOffset.y() > 100)
        {
            if (walking) {
                this.me.Exec(2, "Stop", "forward");
                this.me.Exec(2, "Stop", "back");
            } else
                this.me.Exec(2, "Move", "back");
            this.listenGesture = false;
        }
        gestureEvent.Accept();
    }
    else if (gestureEvent.GestureType() == Qt.PinchGesture)
    {
        var scaleChange = gestureEvent.Gesture().scaleFactor - gestureEvent.Gesture().lastScaleFactor;
        if (scaleChange > 0.1 || scaleChange < -0.1)
            this.ClientHandleMouseScroll(scaleChange * 100);
        gestureEvent.Accept();
    }
}

SimpleAvatar.prototype.ClientHandleKeyboardZoom = function(direction) {
    if (direction == "in") {
        this.ClientHandleMouseScroll(10);
    } else if (direction == "out") {
        this.ClientHandleMouseScroll(-10);
    }
}

SimpleAvatar.prototype.ClientHandleMouseScroll = function(relativeScroll) {
    var attrs = this.me.dynamiccomponent;
    // Check that zoom is allowed
    if (!attrs.GetAttribute("enableZoom"))
        return;

    var avatarCameraDistance = attrs.GetAttribute("cameraDistance");

    if (!this.IsCameraActive())
        return;

    var moveAmount = 0;
    if (relativeScroll < 0 && avatarCameraDistance < 500) {
        if (relativeScroll < -50)
            moveAmount = 2;
        else
            moveAmount = 1;
    } else if (relativeScroll > 0 && avatarCameraDistance > 0) {
        if (relativeScroll > 50)
            moveAmount = -2
        else
            moveAmount = -1;
    }
    if (moveAmount != 0)
    {
        // Add movement
        avatarCameraDistance = avatarCameraDistance + moveAmount;
        // Clamp distance to be between -0.5 and 500
        if (avatarCameraDistance < -0.5)
            avatarCameraDistance = -0.5;
        else if (avatarCameraDistance > 500)
            avatarCameraDistance = 500;

        attrs.SetAttribute("cameraDistance", avatarCameraDistance);
    }
}

SimpleAvatar.prototype.ClientHandleRotate = function(param) {
    if (param == "left") {
        this.rotate = -1;
    }
    if (param == "right") {
        this.rotate = 1;
    }
}

SimpleAvatar.prototype.ClientHandleStopRotate = function(param) {
    if ((param == "left") && (this.rotate == -1)) {
        this.rotate = 0;
    }
    if ((param == "right") && (this.rotate == 1)) {
        this.rotate = 0;
    }
}

SimpleAvatar.prototype.ClientUpdateRotation = function(frametime) {
    var attrs = this.me.dynamiccomponent;
    // Check that rotation is allowed
    if (!attrs.GetAttribute("enableRotate"))
        return;

    if (this.rotate != 0) {
        this.yaw -= this.rotateSpeed * this.rotate * frametime;
        this.me.Exec(2, "SetRotation", this.yaw.toString());
    }
}

SimpleAvatar.prototype.ClientUpdateAvatarCamera = function() {
    var attrs = this.me.dynamiccomponent;
    if (attrs == null)
        return;
    var avatarCameraDistance = attrs.GetAttribute("cameraDistance");

    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return;
    var cameraplaceable = cameraentity.placeable;

    var cameratransform = cameraplaceable.transform;
    cameratransform.rot = new float3(this.pitch, 0, 0);
    cameratransform.pos = new float3(0, this.avatarCameraHeight, avatarCameraDistance);

    cameraplaceable.transform = cameratransform;
}

SimpleAvatar.prototype.ClientHandleMouseMove = function(mouseevent) {
    var attrs = this.me.dynamiccomponent;

    // Do not rotate if not allowed
    if (!attrs.GetAttribute("enableRotate"))
        return;

    // Do not rotate in third person if right mousebutton not held down
    if (input.IsMouseCursorVisible())
        return;

    if (mouseevent.IsRightButtonDown())
        this.LockMouseMove(mouseevent.relativeX, mouseevent.relativeY);
    else
        this.isMouseLookLockedOnX = true;

    var cameraentity = scene.GetEntityByName("AvatarCamera");
    if (cameraentity == null)
        return;

    // Dont move av rotation if we are not the active cam
    if (!cameraentity.camera.IsActive())
        return;

    if (mouseevent.relativeX != 0)
    {
        // Rotate avatar or camera
        this.yaw -= this.mouseRotateSensitivity * parseInt(mouseevent.relativeX);
        this.me.Exec(2, "SetRotation", this.yaw.toString());
    }

    if (mouseevent.relativeY != 0 && !this.isMouseLookLockedOnX)
    {
        // Look up/down
        var attrs = this.me.dynamiccomponent;
        this.pitch -= this.mouseRotateSensitivity * parseInt(mouseevent.relativeY);

        // Dont let the 1st person flip vertically, 180 deg view angle
        if (this.pitch < -90)
            this.pitch = -90;
        if (this.pitch > 90)
            this.pitch = 90;
    }
}

SimpleAvatar.prototype.LockMouseMove = function(x,y) {
    if (Math.abs(y) > Math.abs(x))
        this.isMouseLookLockedOnX = false;
    else
        this.isMouseLookLockedOnX = true;
}

SimpleAvatar.prototype.CommonFindAnimations = function() {
    var animcontrol = this.me.animationcontroller;
    if (animcontrol == null)
        return;
    var availableAnimations = animcontrol.GetAvailableAnimations();
    if (availableAnimations.length > 0) {
        // Detect animation names
        for(var i=0; i<this.animList.length; i++) {
            var animName = this.animList[i];
            if (availableAnimations.indexOf(animName) == -1) {
                // Disable this animation by setting it to a empty string
                print("Could not find animation for:", animName, " - disabling animation");
                this.animList[i] = "";
            }
        }

        // Assign the possible empty strings for
        // not found anims back to the variables
        this.standAnimName = this.animList[0];
        this.walkAnimName = this.animList[1];
        this.flyAnimName = this.animList[2];
        this.hoverAnimName = this.animList[3];

        this.animsDetected = true;
    }
}

SimpleAvatar.prototype.CommonUpdateAnimation = function(frametime) {
    // This function controls the known move animations, such as walk, fly, hover and stand,
    // which are replicated through the animationState attribute of the AnimationController.
    // Only one such move animation can be active at a time.
    // Other animations, such as for gestures, can be freely enabled/disabled by other scripts.

    var attrs = this.me.dynamiccomponent;

    if (!this.animsDetected) {
        return;
    }

    var animcontroller = this.me.animationcontroller;
    var rigidbody = this.me.rigidbody;
    if ((animcontroller == null) || (rigidbody == null)) {
        return;
    }

    if (!attrs.GetAttribute("enableAnimation"))
    {
        // When animations disabled, forcibly disable all running move animations
        // Todo: what if custom scripts want to run the move anims as well?
        for (var i = 0; i < this.animList.length; ++i) {
            if (this.animList[i] != "")
                animcontroller.DisableAnimation(this.animList[i], 0.25);
        }
        return;
    }

    var animName = animcontroller.animationState;

    // Enable animation, skip with headless server
    if (animName != "" && !framework.IsHeadless()) {
        // Do custom speeds for certain anims
        if (animName == this.hoverAnimName) {
            animcontroller.SetAnimationSpeed(animName, 0.25);
        }
        // Enable animation
        if (!animcontroller.IsAnimationActive(animName)) {
            animcontroller.EnableAnimation(animName, true, 0.25, false);
        }
        // Disable other move animations
        for (var i = 0; i < this.animList.length; ++i) {
            if ((this.animList[i] != animName) && (this.animList[i] != "") && (animcontroller.IsAnimationActive(this.animList[i])))
                animcontroller.DisableAnimation(this.animList[i], 0.25);
        }
    }

    // If walk animation is playing, adjust its speed according to the avatar rigidbody velocity
    if (animName != ""  && animcontroller.IsAnimationActive(this.walkAnimName)) {
        var velocity = rigidbody.linearVelocity;
        var walkspeed = Math.sqrt(velocity.x * velocity.x + velocity.z * velocity.z) * this.walkAnimSpeed;
        animcontroller.SetAnimationSpeed(this.walkAnimName, walkspeed);
    }
}

