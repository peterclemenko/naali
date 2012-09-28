// !ref: http://chiru.cie.fi/lvm-assets/Osprey.mesh
// !ref: http://chiru.cie.fi/lvm-assets/Osprey.skeleton
// !ref: http://chiru.cie.fi/lvm-assets/leathers.002.material
// !ref: http://chiru.cie.fi/lvm-assets/body.001.material

/*if (!server.IsRunning() && !framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
}*/

function FlyingAvatar(entity, comp)
{
    this.me = entity;
    this.isServer = server.IsRunning() || server.IsAboutToStart();
    
    if (this.isServer)
        this.ServerInitialize();
    else
        this.ClientInitialize();
}

FlyingAvatar.prototype.OnScriptObjectDestroyed = function() {
    // Must remember to manually disconnect subsystem signals, otherwise they'll continue to get signalled
    if (this.isServer)
        scene.physics.Updated.disconnect(this, this.ClientUpdatePhysics);
    else
        frame.Updated.disconnect(this, this.ClientUpdate);
}

FlyingAvatar.prototype.ServerInitialize = function ()
{
    var mesh = this.me.GetOrCreateComponent("EC_Mesh");
    var meshRef = mesh.meshRef;
    meshRef.ref = "http://chiru.cie.fi/lvm-assets/Osprey.mesh";
    mesh.meshRef = meshRef;
    
    var skeletonRef = mesh.skeletonRef;
    skeletonRef.ref = "http://chiru.cie.fi/lvm-assets/Osprey.skeleton";
    mesh.skeletonRef = skeletonRef;
    
    var materials = mesh.meshMaterial;  
    materials = ["http://chiru.cie.fi/lvm-assets/leathers.002.material", "http://chiru.cie.fi/lvm-assets/body.001.material"];
    mesh.meshMaterial = materials;
    
    var trans = mesh.nodeTransformation;
    trans.rot.y = 180;
    mesh.nodeTransformation = trans;
    
    var rigidbody = this.me.GetOrCreateComponent("EC_RigidBody");
    rigidbody.AssertAuthority(false);
    print("Gave rigidbody authority to client");
}

FlyingAvatar.prototype.ClientInitialize = function()
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    
    this.timer = new QTimer();
    this.currentRoll = 0;
    this.currentYaw = 0;
    this.currentPitch = 0;
    this.stabilize = false;
    this.motionX = 0;
    this.motionY = 0;
    this.motionZ = 0;
    
    this.me.SetTemporary(true);
    
    if (this.me.name == "Flyer" + client.GetConnectionID()) {
        this.ownAvatar = true;
        
        var placeable = this.me.GetOrCreateComponent("EC_Placeable");
        
        var rigidbody = this.me.GetOrCreateComponent("EC_RigidBody");
        rigidbody.AssertAuthority(true);

        rigidbody.mass = 10;
        rigidbody.shapeType = 3; // Capsule
        rigidbody.size = float3(0.5, 0.5, 2.4);
        rigidbody.gravityEnabled = false;

        this.me.Action("Move").Triggered.connect(this, this.ClientHandleMove);
        this.me.Action("Stop").Triggered.connect(this, this.ClientHandleStop);
        this.me.Action("MouseLookX").Triggered.connect(this, this.ClientHandleMouseLookX);
        this.me.Action("MouseLookY").Triggered.connect(this, this.ClientHandleMouseLookY);
        
        var attrs = this.me.dynamiccomponent;
        attrs.CreateAttribute("real", "rollSensitivity");
        attrs.CreateAttribute("real", "rotateSensitivity");
        attrs.CreateAttribute("real", "stabilizationSpeed");
        attrs.CreateAttribute("real", "stabilizationWaitout");
        attrs.CreateAttribute("bool", "invertMouseY");
        attrs.CreateAttribute("real", "maxRollDegree");
        attrs.CreateAttribute("real", "moveSpeed");
        attrs.CreateAttribute("real", "dampingForce");
        attrs.SetAttribute("rollSensitivity", 0.3);
        attrs.SetAttribute("rotateSensitivity", 0.2);
        attrs.SetAttribute("stabilizationSpeed", 5.0);
        attrs.SetAttribute("stabilizationWaitout", 400);
        attrs.SetAttribute("invertMouseY", true);
        attrs.SetAttribute("maxRollDegree", 30.0);
        attrs.SetAttribute("moveSpeed", 30.0);
        attrs.SetAttribute("dampingForce", 1.0);
        
        // Hook to update ticks
        frame.Updated.connect(this, this.ClientUpdate);
        this.timer.timeout.connect(this, this.ClientStartStabilization);
        scene.physics.Updated.connect(this, this.ClientUpdatePhysics);
        rigidbody.PhysicsCollision.connect(this, this.ClientHandleCollision);
        
        this.ClientCreateCamera();
        this.ClientCreateInputMapper();
    }
}

FlyingAvatar.prototype.ClientCreateCamera = function()
{
    var cameraentity = scene.GetEntityByName("FlyingCamera");
    if (cameraentity == null)
    {
        cameraentity = scene.CreateLocalEntity();
        cameraentity.SetName("FlyingCamera");
        cameraentity.SetTemporary(true);
    }

    var camera = cameraentity.GetOrCreateComponent("EC_Camera");
    var placeable = cameraentity.GetOrCreateComponent("EC_Placeable");
    
    camera.SetActive();
    
    var parentRef = placeable.parentRef;
    parentRef.ref = this.me;
    placeable.parentRef = parentRef;
}

FlyingAvatar.prototype.ClientCreateInputMapper = function()
{
    var inputmapper = this.me.GetOrCreateComponent("EC_InputMapper");
    
    inputmapper.contextPriority = 101;
    inputmapper.SetNetworkSyncEnabled(false);
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.keyrepeatTrigger = false;
    inputmapper.executionType = 1;
    inputmapper.RegisterMapping("W", "Move(forward)", 1);
    inputmapper.RegisterMapping("S", "Move(back)", 1);
    inputmapper.RegisterMapping("A", "Move(left)", 1);
    inputmapper.RegisterMapping("D", "Move(right)", 1);
    inputmapper.RegisterMapping("Space", "Move(up)", 1);
    inputmapper.RegisterMapping("C", "Move(down)", 1);
    inputmapper.RegisterMapping("W", "Stop(forward)", 3);
    inputmapper.RegisterMapping("S", "Stop(back)", 3);
    inputmapper.RegisterMapping("A", "Stop(left)", 3);
    inputmapper.RegisterMapping("D", "Stop(right)", 3);
    inputmapper.RegisterMapping("Space", "Stop(up)", 3);
    inputmapper.RegisterMapping("C", "Stop(down)", 3);
    
    var mousemapper = this.me.GetOrCreateComponent("EC_InputMapper", "MouseMapper", 2, false);
    mousemapper.contextPriority = 100;
    mousemapper.SetNetworkSyncEnabled(false);
    mousemapper.takeMouseEventsOverQt = true;
    var inputContext = mousemapper.GetInputContext();
    inputContext.MouseRightReleased.connect(this, this.HandleMouseRightReleased);
}

FlyingAvatar.prototype.ClientStartStabilization = function()
{
    this.stabilize = true;
    this.timer.stop();
}

FlyingAvatar.prototype.ClientStabilizeAvatar = function()
{ 
    if(!this.stabilize)
        return;
        
    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    var stabilizationSpeed = attrs.GetAttribute("stabilizationSpeed");
    
    // Adjust roll towards 
    if(this.currentRoll > 0)
        this.currentRoll -= this.currentRoll / (100 / stabilizationSpeed);
    else if(this.currentRoll < 0)
        this.currentRoll -= this.currentRoll / (100 / stabilizationSpeed);
    
    // Adjust pitch towards zero
    if(this.currentPitch > 0)
        this.currentPitch -= this.currentPitch / (50 / stabilizationSpeed);
    else if(this.currentPitch < 0)
        this.currentPitch -= this.currentPitch  / (50 / stabilizationSpeed);
    
    // Clamp roll
    if(Math.abs(this.currentRoll) < 1)
        this.currentRoll = 0;
    // Clamp pitch
    if(Math.abs(this.currentPitch) < 1)
        this.currentPitch = 0;
    
    if(this.currentPitch == 0 && this.currentRoll == 0)
        this.stabilize = false;
}

FlyingAvatar.prototype.HandleMouseRightReleased = function()
{
    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    var stabilizationWaitout = attrs.GetAttribute("stabilizationWaitout");
    
    this.timer.stop();
    this.timer.start(stabilizationWaitout);
    this.stabilize = false;
}

FlyingAvatar.prototype.ClientHandleMouseLookX = function(param)
{      
    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    var rollSensitivity = attrs.GetAttribute("rollSensitivity");
    var rotateSensitivity = attrs.GetAttribute("rotateSensitivity");
    var maxRoll = attrs.GetAttribute("maxRollDegree");

    // Stop stabilizing while moving
    this.timer.stop();
    this.stabilize = false;
    
    var move = parseInt(param);
    var degrees = param * rollSensitivity; 
    
    // Clamp roll between -maxRoll..maxRoll
    if(!((this.currentRoll >= maxRoll && degrees > 0) || (this.currentRoll <= -maxRoll && degrees < 0))) 
        this.currentRoll += degrees;
    this.currentYaw -= rotateSensitivity * move;
}

FlyingAvatar.prototype.ClientHandleMouseLookY = function(param)
{
    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    var rotateSensitivity = attrs.GetAttribute("rotateSensitivity");
    var invertMouseY = attrs.GetAttribute("invertMouseY");
    
    // Stop stabilizing while moving
    this.timer.stop();
    this.stabilize = false;

    if(invertMouseY)
        param *= -1;
    
    // Clamp pitch between -90..90 to keep the ground beneath us
    if(this.currentPitch < 90 && this.currentPitch > -90)
        this.currentPitch -= rotateSensitivity * param;
}

FlyingAvatar.prototype.ClientUpdateOrientation = function()
{
    var placeable = this.me.GetComponent("EC_Placeable")
    var rigidbody = this.me.rigidbody;
    
    var rollWorld = new Quat(float3(0,0,-1), DegToRad(this.currentRoll));
    var yawWorld = new Quat(float3(0,1,0), DegToRad(this.currentYaw));
    var pitchWorld = new Quat(float3(1,0,0), DegToRad(this.currentPitch));
    
    var orientation = yawWorld.Mul(rollWorld);
    orientation = orientation.Mul(pitchWorld);
        
    rigidbody.SetOrientation(orientation);
}

FlyingAvatar.prototype.ClientUpdateCamera = function()
{
}

FlyingAvatar.prototype.ClientHandleMove = function(param)
{
    if(!this.IsCameraActive())
        return;
    
    if (param == "forward") {
        this.motionZ = -1;
    }
    if (param == "back") {
        this.motionZ = 1;
    }
    if (param == "right") {
        this.motionX = 1;
    }
    if (param == "left") {
        this.motionX = -1;
    }
    if (param == "up") {
        this.motionY = 1;
    }
    if (param == "down") {
        this.motionY = -1;
    }
}

FlyingAvatar.prototype.ClientHandleStop = function(param)
{
    if(!this.IsCameraActive())
        return;
        
    if ((param == "forward") && (this.motionZ == -1)) {
        this.motionZ = 0;
    }
    if ((param == "back") && (this.motionZ == 1)) {
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
}

FlyingAvatar.prototype.ClientUpdate = function(frametime)
{
    if (!this.IsCameraActive())
    {
        motion_x = 0;
        motion_y = 0;
        motion_z = 0;
        return;
    }

    this.ClientStabilizeAvatar()
    this.ClientUpdateCamera();
    this.ClientUpdateOrientation();
}

FlyingAvatar.prototype.ClientUpdatePhysics = function(frametime)
{   
    //print("ClientUpdatePhysics");

    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    var moveSpeed = attrs.GetAttribute("moveSpeed");
    var dampingForce = attrs.GetAttribute("dampingForce");

    var placeable = this.me.placeable;
    var rigidbody = this.me.rigidbody;

    // Apply motion force
    // If diagonal motion, normalize
    if ((this.motionX != 0) || (this.motionY != 0) || (this.motionZ != 0)) {
        var impulseVec = new float3(this.motionX, this.motionY, this.motionZ).Normalized().Mul(moveSpeed);
        var tm = placeable.LocalToWorld();
        impulseVec = tm.MulDir(impulseVec);
        
        rigidbody.ApplyImpulse(impulseVec);
    }

    // Apply damping. Only do this if the body is active, because otherwise applying forces
    // to a resting object wakes it up
    if (rigidbody.IsActive()) {
        var dampingVec = rigidbody.GetLinearVelocity();
        //dampingVec = dampingVec.Mul(-dampingForce);
        dampingVec.x = -dampingForce * dampingVec.x;
        dampingVec.y = -dampingForce * dampingVec.y;
        dampingVec.z = -dampingForce * dampingVec.z;
        rigidbody.ApplyImpulse(dampingVec);
    }
}

FlyingAvatar.prototype.ClientHandleCollision = function(ent, pos, normal, distance, impulse, newCollision)
{
    var attrs = this.me.dynamiccomponent;
    if(!attrs)
        return;
        
    print("ClientHandleCollision");
    
    var stabilizationWaitout = attrs.GetAttribute("stabilizationWaitout");

    rigidbody = this.me.rigidbody;
    rigidbody.SetAngularVelocity(float3(0,0,0));
    
    // Start stabilization timer and apply some additional impulse.
    // Just a quick test to get some kind of a crash effect,
    // should be implemented properly.
    if(newCollision)
    {
        this.timer.stop();
        this.timer.start(stabilizationWaitout);
        this.stabilize = false;
        
        normal.x *= -impulse * 2;
        normal.y *= -impulse * 2;
        normal.z *= -impulse * 2;
        rigidbody.ApplyImpulse(normal);
    }
}

FlyingAvatar.prototype.IsCameraActive = function()
{
    var cameraentity = scene.GetEntityByName("FlyingCamera");
    if (cameraentity == null)
        return false;
    var camera = cameraentity.camera;
    return camera.IsActive();
}

FlyingAvatar.prototype.ServerUpdate = function(frametime)
{
}

function RadToDeg3(value)
{
    return value.Mul(180.0 / Math.PI);
}

function DegToRad3(value)
{
    return value.Mul(Math.PI / 180.0);
}

function RadToDeg(value)
{
    return value * (180.0 / Math.PI);
}

function DegToRad(value)
{
    return value * (Math.PI / 180.0);
}
