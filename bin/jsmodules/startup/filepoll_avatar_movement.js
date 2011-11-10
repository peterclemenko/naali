engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function poll_file(frametime) {
    var entity = scene.GetEntity(avatarEntityId);
    if (entity == null)
    {
        frame.Updated.disconnect(ProduceRandomMovement);
        return;
    }

    var f = new QFile("/tmp/kek");
    if (!f.exists())
	return;

    if (!f.open(QIODevice.ReadOnly)) {
	print("failed to open poll file");
	return;
    }
    var ts = new QTextStream(f);
    var line = ts.readLine();
    print ("filepoll: file contained: " + line);
    
    f.remove();
    print ("removed file");

    var words = line.split(/\s/);
    words.filter(function (w) { return w != ""; }); // remove empty strings

    // var action_name = words[0];
    // words.splice(0, 1); // remove first element
    entity.Exec(2, words[0], words[1]);
    
}

if (!server.IsRunning())
    frame.Updated.connect(poll_file);

framework.Scene().SceneAdded.connect(OnSceneAdded);

var scene = null;
var avatarEntityId = 0;

function OnSceneAdded(scenename)
{
    // Get pointer to scene through framework
    scene = framework.Scene().GetScene(scenename);
    scene.EntityCreated.connect(OnEntityCreated);
}

function OnEntityCreated(entity, change)
{
    if (!server.IsRunning() && entity.name == "Avatar" + client.GetConnectionID())
    {
        avatarEntityId = entity.id;
        frame.Updated.connect(poll_file);
    }
}

