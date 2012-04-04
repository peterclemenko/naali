cxserver_hostname = "chiru.cie.fi";
cxserver_port = 2921;

SKIP_SELF = True # use False for testing on single machine, set to
                  # True for real use

import socket, threading, cgi, tundra, json
from PythonQt.QtCore import QByteArray

def socket_readline(s):
    buf = ""
    while 1:
        x = s.recv(512)
        if not x: # eof
            break

        buf += x
        
        if buf.endswith("\r\n"):
            break
    return buf


def tcp_send(msg, host, port):
    #tundra.Console().LogInfo("tcp_start connecting to %s:%s" % (host, port))
    print "tcp_start connecting"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if not msg.endswith("\r\n"):
        msg += "\r\n"
    s.connect((host, port))
    s.sendall(msg)
    #tundra.Console().LogError("bad " + str(what))
    #tundra.Console().LogInfo("ok")
    resp = socket_readline(s)
    s.close()
    tundra.Console().LogInfo("tcp_start chat with %s:%s -> %s" % (host, port, repr(resp)))
    return resp

def errwrap(fun, *args, **kw):
    try:
        return apply(fun, args, kw)
    except:
        import traceback
        tb = traceback.format_exc()
        args = 'args = ' + repr(args) + '\n'
        tundra.Console().LogError("error in socket io thread:\n" + args + tb)

def tcp_send_thread(msg, host, port):
    f = lambda: tcp_send(msg, host, port)
    t = threading.Thread(target=errwrap(f))
    t.setDaemon(True)
    t.start()

def on_sceneadded(scenename):
    scene = tundra.Scene().GetSceneRaw(scenename)
    throw_ent = scene.CreateEntityLocalRaw([]);
    throw_ent.SetName("Throwing");
    throw_ent.SetTemporary(True);
    throw_ent.Action("CatchObject").connect("Triggered(QString, QString, QString, QStringList)", on_catch)
    throw_ent.Action("ThrowObject").connect("Triggered(QString, QString, QString, QStringList)", on_throw)
    # if tundra.IsServer():
    #     tundra.Frame().DelayedExecute(.5).connect(
    #         'Triggered(float)', lambda t: throw_ent.Exec(1, "ThrowObject", "Fish", scenename))

def register_to_context(cxname, data, callback):
    h, p = cxserver_hostname, cxserver_port
    qt_http_post("http://%s:%s/register" % (h, p), data, callback)
    

def get_context_others(cxname, callback):
    h, p, c = cxserver_hostname, cxserver_port, cgi.escape(cxname)
    qt_http_get("http://%s:%s/members/%s" % (h, p, c), callback)
    
def throw_to(endpoint, msg):
    msg_string = json.dumps(msg)
    host, port = endpoint
    tcp_send_thread(msg_string, host, port)


def on_throw(entname, scenename):
    scene = tundra.Scene().GetSceneRaw(scenename)
    if not scene:
        tundra.Console().LogError("throw: scene %s not found" % repr(scenename))
    e = scene.GetEntityByNameRaw(entname)
    if not e:
	tundra.Console().LogError("OnThrow: entity %s not found" % repr(entname))
	return

    def getxyz(f3):
        return f3.x(), f3.y(), f3.z()

    p = e.placeable
    po, s, o = map(getxyz, [p.Position(), p.Scale(), p.Orientation().QuatToEulerZYX()])
    transform_str = ', '.join(map(str, po+s+o))
    msg = dict(op="action", exectype=1, scene=None,
               entity="Throwing",
               action="CatchObject",
               params=[entname, e.mesh.meshRef.ref(), transform_str])
    
    def on_context_reg_response(data):
        print "context reg ok"
        get_context_others("throwdemo", on_context_query_response)
        

    def on_context_query_response(data):
        
        tundra.Console().LogInfo("throw stage2 start")
        try:
            who = json.loads(data)
        except ValueError:
            tundra.Console().LogError("context query response parse error: bad json data: " + repr(data))
            return

        for guy_id, guy_info in who.items():
            if SKIP_SELF and guy_id == my_id():
                print "skipping self"
                continue
            else:
                print "throwing to", guy_id
                throw_to(guy_info["json_endpoint"], msg)
            
        tundra.Console().LogInfo("throw stage2 done")

    reginfo = dict(context_id="throwdemo", member_id=my_id(), json_endpoint=(my_id(), 4242))
    register_to_context("throwdemo", reginfo, on_context_reg_response)

    tundra.Console().LogInfo("throw stage1 success")


def my_id():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('8.8.8.8', 53))
    return s.getsockname()[0]

def on_catch(caught_name, mesh_url, transform_str):
    scene = tundra.Scene().MainCameraScene()
    caught_ent = scene.CreateEntityLocalRaw(["EC_Mesh", "EC_Placeable"]);
    caught_ent.SetName(caught_name)
    caught_ent.mesh.SetMeshRef(mesh_url)
    caught_ent.mesh.SetMaterial(0, mesh_url.replace('.mesh', '.material'))
    t = map(float, transform_str.split(', '))
    caught_ent.placeable.SetPosition(*t[:3])
    scale = caught_ent.placeable.SetScale(*t[3:6])
    o = caught_ent.position.Orientation()
    o.FromEulerXYZ(*t[6:9])
    caught_ent.position.SetOrientation(o)

from PythonQt.QtNetwork import QNetworkRequest, QNetworkAccessManager
from PythonQt.QtCore import QUrl

qnam_get = QNetworkAccessManager()
qnam_post = QNetworkAccessManager()

def qt_http_get(url, callback):
    req = QNetworkRequest(QUrl(url))
    def on_finish(reply):
        if reply.error():
            print "qnam http reply error:", reply.error()
            return
        else:
            data = reply.readAll().data()
            print 'qnam got:', repr(data)
            if data:
                callback(data)

    qnam_get.connect("finished(QNetworkReply*)", on_finish)
    qnam_get.get(req)

def qt_http_post(url, datadict, callback):
    req = QNetworkRequest(QUrl(url))
    def on_finish(reply):
        if reply.error():
            print "qnam http reply error:", reply.error()
            return
        else:
            data = reply.readAll().data()
            print 'qnam got:', repr(data)
            if data:
                callback(data)
            
    qnam_post.connect("finished(QNetworkReply*)", on_finish)
    qnam_post.post(req, QByteArray(json.dumps(datadict)))


if 1: #tundra.IsServer():
    tundra.Scene().connect("SceneAdded(QString)", on_sceneadded)
