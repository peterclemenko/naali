if (!server.IsRunning() && !framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
}

var lastX = 0;
var lastY = 0;
var distance = 0;

function ObjectGrab(entity, comp)
{
    this.me = entity;
    this.selectedId = -1;
    print("Lals");
    //this.originalPosition = new float3();
    //this.originalOrientation = new Quat();
    this.originalTransform = 0;
    this.originalMass = 0;
    this.animDirection = true;
    
    if(!server.IsRunning())
    {
        frame.Updated.connect(this, this.UpdateSelectionAnimation);
        this.CreateInput();
    }
}

ObjectGrab.prototype.CreateInput = function()
{
    // Use inputmapper with mouse for now, since
    // we don't have the fancy sensors working yet
    var inputmapper = this.me.GetOrCreateComponent("EC_InputMapper", 2, false);
    inputmapper.contextPriority = 100;
    inputmapper.takeMouseEventsOverQt = true;
    inputmapper.modifiersEnabled = false;
    inputmapper.executionType = 1; // Execute actions locally
    
    // Connect mouse gestures
    var inputContext = inputmapper.GetInputContext();
    //inputContext.GestureStarted.connect(this, this.GestureStarted);
    //inputContext.GestureUpdated.connect(this, this.GestureUpdated);
    inputContext.MouseScroll.connect(this, this.HandleMouseScroll);
    inputContext.MouseMove.connect(this, this.HandleMouseMove);
    inputContext.MouseLeftPressed.connect(this, this.HandleMouseLeftPressed);
    inputContext.MouseLeftReleased.connect(this, this.HandleMouseLeftReleased);
}

// Get entity id as projected through viewport
// params: screen co-ordinates
// return: entity id if found, -1 for not found
ObjectGrab.prototype.GetTargetedEntity = function(x, y)
{
    var raycastResult = scene.ogre.Raycast(x, y, 0xffffffff);
    if(raycastResult.entity != null) {
        return raycastResult.entity.id;
    }
    return -1;
}

// Set entity as selected
ObjectGrab.prototype.SelectEntity = function(entityId)
{
    var entity = scene.GetEntity(entityId);
    
    // Shouldn't be null, but let's stay on the safe side
    if(entity.mesh == null || entity.placeable == null)
        return;
    
    // Selection changed
    if(this.selectedId != entityId)
    {
        // Release previous selection in case it was left selected
        this.ReleaseSelection();
        this.selectedId = entityId;
    }
}

// Release selection on entity
ObjectGrab.prototype.ReleaseSelection = function()
{
    // Reset the orientation of the old selection
    if(this.selectedId != -1)
    {
        var entity = scene.GetEntity(this.selectedId);
        if (entity.GetComponent("EC_RigidBody"))
        {
            entity.rigidbody.mass = this.originalMass;
        }
        this.selectedId = -1;

    }
}

// If there's a selected object, update the animation indicating
// it's selected
ObjectGrab.prototype.UpdateSelectionAnimation = function()
{
    var entity = scene.GetEntity(this.selectedId);
    if(entity == null)
        return;
    
    var degs = 5;
    
    var transform = entity.placeable.transform;
    transform.rot.y += degs;
    //    transform.rot.x -= degs/2;
    transform.rot.z += degs/2;
    entity.placeable.transform = transform;
}

// Should be connected to selected input method
// Tested to work with mouse, should work with touch but might not work with
// freehand gestures.
ObjectGrab.prototype.MoveSelectedObject = function(X, Y)
{
    var selectedEntity = scene.GetEntity(this.selectedId);
    if(selectedEntity == null)
        return;

    var oldTransform = selectedEntity.placeable.transform;
    oldTransform.pos = scene.ogre.RaycastGetPoint(distance);
    selectedEntity.placeable.transform = oldTransform;
}

// <MOUSE HANDLERS>
ObjectGrab.prototype.HandleMouseScroll = function(event)
{
    if (!event.IsLeftButtonDown())
        return;
    if (event.relativeZ < 0)
        distance -= 1;
    else
        distance += 1;
    var selectedEntity = scene.GetEntity(this.selectedId);
    if(selectedEntity == null)
        return;

    // Update position for the object.
    var oldTransform = selectedEntity.placeable.transform;
    oldTransform.pos = scene.ogre.RaycastGetPoint(distance);
    selectedEntity.placeable.transform = oldTransform;
}

ObjectGrab.prototype.HandleMouseMove = function(event)
{
    if (event.IsLeftButtonDown())
        this.MoveSelectedObject(event.x, event.y);
}

ObjectGrab.prototype.HandleMouseLeftPressed = function(event)
{
    var entityId = this.GetTargetedEntity(event.x, event.y);
    if(entityId == -1)
        return;
    // Allow only dices to be grabbed in the scene.
    if(entityId <16 || entityId > 18)
        return;

    this.SelectEntity(entityId);
    var cameraId = GetActiveCameraId();
    var cameraEntity = scene.GetEntity(cameraId);
    var selectedEntity = scene.GetEntity(this.selectedId);
    var cameraPosition = cameraEntity.placeable.transform.pos;
    var selectedPosition = selectedEntity.placeable.transform.pos;

    distance = cameraPosition.Distance(selectedPosition);
    // Set mass to 0 before moving so gravity wont hinder movement.
    if (selectedEntity.GetComponent("EC_RigidBody"))
    {
        this.originalMass = selectedEntity.rigidbody.mass;
        selectedEntity.rigidbody.mass = 0;
    }
}

ObjectGrab.prototype.HandleMouseLeftReleased = function(event)
{
    this.ReleaseSelection(this.entityId);
}
// </MOUSE HANDLERS>

function GetActiveCameraId()
{
    // Hax for now..
    var freelookcameraentity = scene.GetEntityByName("FreeLookCamera");
    var avatarcameraentity = scene.GetEntityByName("AvatarCamera");
    
    if(freelookcameraentity != null && freelookcameraentity.camera.IsActive())
        return freelookcameraentity.id;
    if(avatarcameraentity != null && avatarcameraentity.camera.IsActive())
        return avatarcameraentity.id;
    return -1;
}

function DegToRad(deg)
{
    return deg * (Math.PI / 180.0);
}
