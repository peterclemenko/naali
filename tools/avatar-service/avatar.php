<?php include_once "top.php"; ?>

<?php
if ($_SESSION["logged_in"]==true){
    //get avatar model filename
    include_once "action/dbconnect.php";
    $avatarid = $_GET['avatar'];
    $modelsDir = 'scene/models/';
    $query = 'SELECT * FROM avatar WHERE avatarId=' . $avatarid;
    $result = mysql_query($query);
    /*if(!$result){
        print mysql_error();
        mysql_close($dbConnection);
        exit;
    }*/
    $dataArray = mysql_fetch_assoc($result);
    $file = $dataArray['avatarFile'];
    $folder = $dataArray['avatarName'] . "/models/";
    $avatar = $modelsDir . $folder . $file;
    $scale = $dataArray['avatarScale'];
    $floormodel = $modelsDir . "floor.dae";
    
    include_once "action/dbdisconnect.php";
}
?>

<script language="javascript">
function OnChange(dropdown){
    var myindex  = dropdown.selectedIndex
    var selectValue = dropdown.options[myindex].value
    var baseURL  = "avatar.php?avatar=" + selectValue
    top.location.href = baseURL;
    return true;
}
</script>

<!-- GLGE -->
<script type='text/javascript' src='glge/src/core/glge_math.js'></script> 
<script type='text/javascript' src='glge/src/core/glge.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_event.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_quicknote.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_animatable.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_document.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_placeable.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_jsonloader.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_group.js'></script> 
<script type='text/javascript' src='glge/src/core/glge_messages.js'></script> 
<script type='text/javascript' src='glge/src/animation/glge_action.js'></script> 
<script type='text/javascript' src='glge/src/animation/glge_actionchannel.js'></script> 
<script type='text/javascript' src='glge/src/animation/glge_animationcurve.js'></script> 
<script type='text/javascript' src='glge/src/animation/glge_animationvector.js'></script> 
<script type='text/javascript' src='glge/src/animation/glge_animationpoints.js'></script> 
<script type='text/javascript' src='glge/src/geometry/glge_mesh.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_material.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_materiallayer.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_multimaterial.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_texture.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_texturecamera.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_texturecanvas.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_texturecube.js'></script> 
<script type='text/javascript' src='glge/src/material/glge_texturevideo.js'></script> 
<script type='text/javascript' src='glge/src/renderable/glge_lod.js'></script> 
<script type='text/javascript' src='glge/src/renderable/glge_object.js'></script> 
<script type='text/javascript' src='glge/src/renderable/glge_text.js'></script> 
<script type='text/javascript' src='glge/src/renders/glge_renderer.js'></script> 
<script type='text/javascript' src='glge/src/scene/glge_camera.js'></script> 
<script type='text/javascript' src='glge/src/scene/glge_light.js'></script> 
<script type='text/javascript' src='glge/src/scene/glge_scene.js'></script> 
<script type='text/javascript' src='glge/src/extra/glge_particles.js'></script> 
<script type='text/javascript' src='glge/src/extra/glge_filter2d.js'></script> 
<script type='text/javascript' src='glge/src/extra/filters/glge_filter_glow.js'></script> 
<script type='text/javascript' src='glge/src/extra/filters/glge_filter_ao.js'></script> 
<script type='text/javascript' src='glge/src/extra/glge_collada.js'></script> 
<script type='text/javascript' src='glge/src/extra/glge_input.js'></script> 
<script type='text/javascript' src='glge/src/extra/glge_wavefront.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsext.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsabstract.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsbox.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsmesh.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsplane.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicssphere.js'></script> 
<script type='text/javascript' src='glge/src/physics/glge_physicsconstraintpoint.js'></script> 

<!-- jQuery --> 
<script type='text/javascript' src='js/jquery-1.6.1.js'></script> 

<!-- "Settings" --> 
<script type='text/javascript'>
    url_to_static = '<?php echo $floormodel; ?>';
    websocket_host = 'localhost';
    websocket_port = '9999';
</script>

<script type='text/javascript'>

/*
  global.js - Stuff to handle things. Handles general avatar stuff.
*/

var canvas;
var width;
var height;
var myid;
var timerid;

var moves = new Array()
var old_moves = new Array()


var rightMouseDown = false;
var old_mousex = 0.0;
var old_mousey = 0.0; 

var handlers = {};

function setId() {
    myid = arguments[0]['id'];
}

function addmove(move) {
    moves.push(move)
}

function isin(value, list) {
    for (i = 0; i < list.length; i++) {
	if (value == list[i]) {
	    return true;
	}
    }
    return false;
}

function checkmove() {

    /*
      Does not work well except for walking and stopping. Need to
      rethink how keyboard mapping should be done.
     */

    for (i in moves) {
	var move = moves[i];
	if (!(isin(move, old_moves))) {
	    action = move.split(',')[0];
	    dir = move.split(',')[1];
	    //console.log('MOVE ' + move);
	    ws.send(JSON.stringify(["Action", {action: action, params: [dir], id: myid}]));
	}
    }

    for (i in old_moves) {
	var move = old_moves[i];
	if (!(isin(move, moves))) {
	    action = move.split(',')[0];
	    if (action == 'Rotate') {
		action = 'StopRotate';
	    } else if (action == 'Move') {
		action = 'Stop';
	    }
	    
	    dir = move.split(',')[1];

	    //console.log('STOP ' + move);
	    ws.send(JSON.stringify(["Action", {action: action, params: dir, id: myid}]));
	}
    }

    old_moves = moves.slice(0);
    moves = [];
}

function sendSignal(signal) {
    var action = signal.split(':')[0];
    var id = signal.split(':')[1];
    //console.log('sending: ' + action + ' to ' + id);
    
}

function connectHandler(signal, id) {
    handlers[signal] = handlers[signal] || [];
    handlers[signal].push(id);
}
/*
  3d.js - Drawing avs in three dimensions. Handles input also.
*/

var scene;
var renderer;
var camera;
var keys;
var mouse;
var hoverobject;
var mouseovercanvas;
var lasttime=0;
var frameratebuffer=60;
var start=parseInt(new Date().getTime());
var now;

function initGraffa() {
    canvas = document.getElementById('graffa');

    //disable context menu for canvas
    canvas.oncontextmenu = function(event) {
        return false;
    }

    renderer = new GLGE.Renderer(canvas);
    scene = new GLGE.Scene();


    // temp testing
    var static_scene = new GLGE.Collada();
    static_scene.setDocument(url_to_static);
    static_scene.docURL = "/";
    static_scene.setRot(0, 0, 0);
    scene.addCollada(static_scene);

    keys = new GLGE.KeyInput();

    scene.setAmbientColor('#fff');
    
    renderer.setScene(scene);

    camera = new GLGE.Camera();
    camera.setType(GLGE.C_PERSPECTIVE);
    camera.setAspect(1/2);

    scene.setCamera(camera); 
    startRender()
}

function startRender() {
    rendertimerid = setInterval(render, 50);
}

function render() {
    renderer.render();
    checkkeys();

    now=parseInt(new Date().getTime());
    frameratebuffer = Math.round(((frameratebuffer * 9) + 1000/ (now - lasttime)) / 10);
    //document.getElementById("fps").innerHTML = "FPS: " + frameratebuffer+ " #obj: " + scene.getObjects().length;

    //document.getElementById("info").innerHTML="Camera:" + camera.getLocX() +", " + camera.getLocY() + ", " + camera.getLocZ() + " : " + camera.getRotX() + ", " + camera.getRotY() + ", " + camera.getRotZ();

    lasttime = now;
}

function checkkeys() {
    //rotation direction from the cameras view
    if (keys.isKeyPressed(GLGE.KI_LEFT_ARROW)) {
        addmove('Rotate,right');
    }
    if (keys.isKeyPressed(GLGE.KI_RIGHT_ARROW)) {
        addmove('Rotate,left');
    }

    checkmove();

}
/* Entity stuff is here
*/

entities = {};

function Entity(id) {
    this.id = id;
    this.components = new Array();

    this.addComponent = function(component) {
	this.components.push(component);
    }
}

(function (Components, $, undefined) {

    Components.EC_Placeable = function(params) {
	this.componentName = 'EC_Placeable';
	this.parent = params['id'];
	//console.log(params)
	this.transform = params['Transform']
    }
    
    Components.EC_Mesh = function(params) {
	this.componentName = 'EC_Mesh';

	this.parent = params['id'];
	
	this.url = params['url']
	//FIXME

    }

    Components.EC_Avatar = function(params) {
	    this.componentName = 'EC_Avatar';
	    var id = params['id'];
	    this.parent = id;

	    this.url = "<?php echo $avatar; ?>";
	
	    if (this.url) {
	        this.mesh = new GLGE.Collada();
	        this.mesh.setId(this.parent);
	        this.mesh.setDocument(this.url);
	        this.mesh.setScale("<?php echo $scale; ?>");
	        scene.addCollada(this.mesh);
	    }
    }
    
}(window.Components = window.Components || {}, jQuery));

function addEntity(params) {
    var id = params['id'];
    if (!entities[id])
	entities[id] = new Entity(id);
}

function removeEntity(params) {
    var id = params['id'];
    for (c = 0; c < scene.children.length; c++) {
        if (scene.children[c].getId() == id) {
            scene.children.splice(c, 1);
            break;
        }
    }
    delete entities[id];
}

function addComponent(params) {
    id = params['id']
    var newComponent = params['component'];
    var component;


    // WS does not have any fancy sync state stuff so if we get an
    // addcomponent message for an entity that hasn't been created yet
    // we'll just add a new entity. This is not the way to go.
    if (!entities[id]){
        addEntity({id: id})
    }
    
    if (Components[newComponent]) {
        //FIXME check that entity does not already have a mesh. Should be done smarter
        for (i = 0; i < entities[id].components.length; i++) {
            if (entities[id].components[i].componentName == newComponent) {
                return;
            }
        }
        component = new Components[newComponent](params);
        entities[id].addComponent(component);
    }
}
    
function setAttr(params) {
    var id = params['id'];
    var component = params['component'];

    //console.log(id + ' SETTING ' + component + ' ' + JSON.stringify(params))
    var comp;

    for (comp in entities[id].components) {
        if (entities[id].components[comp].componentName == component) {
            //console.log('FOUND corresponding component')

	        //Joins components
            jQuery.extend(entities[id].components[comp], params);
            if (component == 'EC_Placeable') {
                //console.log('IS PLACEABLE')
                for (child in scene.children) {
                    var collada = scene.children[child];
                    if (collada.getId() == id) {
                        var transform = params['Transform'];
                        var x = transform[0];
                        var y = transform[2];
                        var z = -transform[1];
                        var rotx = transform[3] * Math.PI / 180;
                        var roty = transform[4] * Math.PI / 180;
                        var rotz = transform[5] * Math.PI / 180;
			
                        collada.setLocX(x);
                        collada.setLocY(y);
                        collada.setLocZ(z);
                        collada.setRotX(rotx);
                        collada.setRotY(rotz - 3 * Math.PI / 2);
                        collada.setRotZ(roty);

                        if (id == myid) {
                            camera.setLoc(x + 3, y + 0.83, z);
                            camera.setRot(0, 0 - 3 * Math.PI / 2, 0);
			            }
		            }
		        }
	        }
	    }
    }
}

function getAttr(params) {
    var id = params["id"];
    var component = params["component"];
    var keys = params["keys"];
    
    var values = []
    for (comp in entities[id].components) {
        if (entities[id].components[comp].componentName == component) {
            for (key in keys) {
                values.push(entities[id].components[comp][keys[key]]);
            }
        }
    }
    return values
}

function loadScene(params) {
    var xmlstring = params['xml'];
    var scenexml = (new DOMParser()).parseFromString(xmlstring, "text/xml");

    var data = {};

    var loadentities = scenexml.getElementsByTagName("entity")

    for (e = 0; e < loadentities.length; e++) {
        var entity = loadentities[e];

        var id = entity.getAttribute("id");
        data[id] = {};
	
        components = entity.getElementsByTagName("component");

        for (c = 0; c < components.length; c++) {
            var component = components[c].getAttribute("type")

            data[id][component] = {};
            var attributes = components[c].getElementsByTagName("attribute");

            for (a = 0; a < attributes.length; a++) {
                var attribute = attributes[a];

                var name = attribute.getAttribute("name");
                var value = attribute.getAttribute("value");
                data[id][component][name] = value
	        }
	    }
    }

    //Add/update components
    // Just use EC_Placeable and EC_MESH for time being
    for (id in data) {
        //add entity (if not in scene)
        if (!entities[id]) {
            addEntity({id: id});
        } 
        else {
            console.log('ERROR: ' + id + ' already in scene!');
        }
        for (component in data[id]) {
            //console.log(component)
            if (component == 'EC_Placeable') {
                //console.log('PLACE');
                addComponent({id: id, component: component, transform: data[id][component]['Transform'].split(',')});
            }
            else if (component == 'EC_Mesh') {
             	//console.log('MESH');
             	addComponent({id: id, component: component, url: data[id][component]['Mesh ref']});
            }
            else if (component == 'EC_Avatar') {
                //console.log('AVATAR');
                addComponent({id: id, component: component});
            }
        }
    }
}
/* Socket.js
Stuff to handle WebSocket communication 
*/

var ws = new WebSocket("ws://" + websocket_host + ":" + websocket_port);

ws.onopen = function() {
    var data = ["CONNECTED", {}];
    ws.send(JSON.stringify(data));
    console.log("Connected");
};

ws.onmessage = function (evt) {
    //console.log('Got message: ' + evt.data);
    parseMessage(evt.data);
};

ws.onclose = function(evt) {
    console.log("Connection closed.");
};

function sendSize(width, height) {
    var data = ["setSize", {width: width, height: height}];
    ws.send(JSON.stringify(data));
}

function parseMessage(message) {
    var message_json = JSON.parse(message);
    var func = message_json[0];
    var params = message_json[1];
    eval(func)(params);
}

function errorMsg(message) {
    ws.send('["ERROR", '+ message +']');
}

function logMessage() {
    console.log('logMessage: ' + JSON.stringify(arguments[0]['message']));
}

function updateAttr() {
    /* Don't ask for update if server hasn't set the id yet */
    /* Currently only syncs EC_Placeable */

    if (myid) {
    var values = getAttr({'id': myid, 'component': 'EC_Placeable', 'keys': ['x', 'y', 'z', 'rotx', 'roty', 'rotz']});
    var data = ["setAttr", {id: myid, component: "EC_Placeable", x: values[0], y: values[1], z: values[2], rotx: values[3], roty: values[5], rotz: values[4]}];
    ws.send(JSON.stringify(data));
    }
}


function reboot() {
    var data = ['reboot', {}];
    clearInterval(sockettimerid);
    ws.send(JSON.stringify(data));
}

function updateObject(id, newdata) {
    var data = ['updateObject', {id: id, data: newdata}];
    ws.send(JSON.stringify(data));
}

//sockettimerid = setInterval(updateAttr, 50);
</script>
<?php
    if ($_SESSION["logged_in"]==true){
        $user_name = $_SESSION["userName"];
        $_SESSION["selectedAvatarId"] = $_GET['avatar'];
        $selected = $_GET['avatar'];
        include_once "action/dbconnect.php";
        
        $query = "SELECT * FROM avatar";
        $result = mysql_query($query);
        if(!$result){
            print mysql_error();
            mysql_close($dbConnection);
            exit;
        }
    
        print "
            <h3>Edit Identity & Avatar</h3>
            <div id='left'>
                <canvas id='graffa' width='250' height='500'>
                    This text is displayed if your browser does not support HTML5 Canvas.
                </canvas>
                <p>Use arrow keys to rotate the avatar</p>
            </div>
            <div id='right'>
                <br />
                <br />
                <form method='post' action='action/saveavatar.php'>
                    Select avatar appearance:<br /><br />
                    <select name='drbavatar' onchange='OnChange(this.form.drbavatar);'>
                    <option value='0'>Select Appearance</option>
        ";
        while ($dataArray = mysql_fetch_assoc($result)){
            if($dataArray['avatarId'] == $selected){
                print "<option value='$dataArray[avatarId]' selected='selected'>$dataArray[avatarName]</option>";
            }
            else{
                print "<option value='$dataArray[avatarId]'>$dataArray[avatarName]</option>";
            }
        }
        print "
                    </select><br />
                    <br />
                    <input type='submit' value='Save changes' />
                </form>
            </div>
        ";
    }
    else{
        print "<p>You must <a href='index.php'>log in</a> to enter this page</p>";
    }
?>
<?php include_once "bottom.php"; ?>
