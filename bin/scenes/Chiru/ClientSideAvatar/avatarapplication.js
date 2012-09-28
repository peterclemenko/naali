// Client-side avatar application, based on the bundled Avatar app.

// This runs in a single EC on the server, and gives each connection
// an avatar entity of their own (and an along with an avatar entity
// script, avatar_entity.js)

engine.IncludeFile("av_common.js");

var avatar_area_size = 10;
var avatar_area_x = 0;
var avatar_area_y = 5;
var avatar_area_z = 0;

is_server = server.IsRunning() || server.IsAboutToStart();
is_client = !is_server;

if (is_client) {
    client.Disconnected.connect(HandleClientDisconnected);
} else if (is_server) {
    server.UserAboutToConnect.connect(ServerHandleUserAboutToConnect);
    server.UserConnected.connect(ServerHandleUserConnected);
    server.UserDisconnected.connect(ServerHandleUserDisconnected);
    
    // If there are connected users when this script was added, add av for all of them
    // TODO new, check

    var userIdList = server.GetConnectionIDs();
    if (userIdList.length > 0)
        log("Application started. Creating avatars for logged in clients.");

    for (var i=0; i < userIdList.length; i++)
    {
        var userId = userIdList[i];
        var userConnection = server.GetUserConnection(userId);
        if (userConnection != null)
            ServerHandleUserConnected(userId, userConnection);
    }
}

function HandleClientDisconnected() {
    // clear up stuff that might be stale after disconnection

    clear_av_connectionids(scene);
}

function ServerHandleUserAboutToConnect(connectionID, user) {
    // Uncomment to test access control
    //if (user.GetProperty("password") != "xxx")
    //    user.DenyConnection();
}

function ServerHandleUserConnected(connectionID, user) {
    if (!user) {
	log("got UserConnected but no user");
    }
    var username = user.GetProperty("username");
    var avatarEntityName = "Avatar_" + username;
    var avatarEntity = scene.GetEntityByName(avatarEntityName);

    // avatar entity state might be out of sync 
    if (!avatarEntity) {
	log("no existing av ent found by username" + username + ", creating new");
        avatarEntity = CreateAvatarEntity(username, connectionID, avatarEntityName);
    } else {
	log("skip setting appearance");
	//SetAvatarAppearance(avatarEntity, "default");
    }
    dc_set(avatarEntity, "connectionID",  connectionID);
    // log("set avatar connid time=" + new Date().getTime());
    // var dc = avatarEntity.GetOrCreateComponent("EC_DynamicComponent");
    // log("read back (2) attribute: " + dc.GetAttribute("connectionID"));
    // log("value: " + dc_get(avatarEntity, "connectionID"));
}

function CreateAvatarEntity(username, connectionID, avatarEntityName) {
    // Create necessary components to the avatar entity:
    // - Script for the main avatar script simpleavatar.js
    // - Placeable for position
    // - AnimationController for skeletal animation control
    // - DynamicComponent for holding disabled/enabled avatar features

    var avatarEntity = scene.CreateEntity(scene.NextFreeId() /*scene.NextFreeIdPersistent()*/, ["EC_Script", "EC_Placeable", "EC_AnimationController", "EC_DynamicComponent"]);
    //avatarEntity.SetKeepOverDisconnect(true);

    dc_set(avatarEntity, "foo", "x");

    avatarEntity.SetTemporary(true); // We never want to save the avatar entities to disk.
    avatarEntity.SetName(avatarEntityName);
    
    avatarEntity.SetDescription(username);

    var script = avatarEntity.script;
    script.className = "AvatarApp.SimpleAvatar";

    // Simpleavatar.js implements the basic avatar movement and animation.
    // Also load an additional script object to the same entity (ExampleAvatarAddon.js) to demonstrate adding features to the avatar.
    var script2 = avatarEntity.GetOrCreateComponent("EC_Script", "Addon", 0, true);
    script2.className = "AvatarApp.ExampleAvatarAddon";

    // Set random starting position for avatar
    var placeable = avatarEntity.placeable;
    var transform = placeable.transform;
    transform.pos.x = (Math.random() - 0.5) * avatar_area_size + avatar_area_x;
    transform.pos.y = avatar_area_y;
    transform.pos.z = (Math.random() - 0.5) * avatar_area_size + avatar_area_z;
    placeable.transform = transform;

    if (username)
        log("Created avatar for " + username);
    return avatarEntity;
}

function ServerHandleUserDisconnected(connectionID, user) {
    username = user.GetProperty("username");
    if (!username) {
	log("no username!");
	return;
    }
    var avatarEntityName = "Avatar_" + username;
    var avatarEntity = scene.GetEntityByName(avatarEntityName);
    if (avatarEntity != null) {
	// not removing avatar entity. will stick around
	log("clearing connectionid from " + avatarEntityName);
	log("dc 2");
	dc_set(avatarEntity, "connectionID", "");
	log("connectionID now: '" + dc_get(avatarEntity, "connectionID") + "'");

        var av_transform = avatarEntity.placeable.transform;
        var entityID = avatarEntity.id;

        if (user != null) {
	    log("User " + username + " disconnected, destroyed avatar entity.");
	    log("dc 3");
            dc_set(me, username, av_transform);
        }
    }
}
