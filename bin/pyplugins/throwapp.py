cxserver_hostname = "chiru.cie.fi";
cxserver_port = 16865;

import socket, threading, cgi

def socket_readline(s):
    data = ""
    while 1:
        data += s.recv(512)
        if data.endswith("\r\n"):
            break
    return data


def tcp_send(msg, host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.sendall(msg)
    resp = socket_readline(s)
    print "socket io: response " + repr(resp)

def tcp_send_thread(msg, host, port):
    t = threading.Thread(target=tcp_send, args=(msg, host, port))
    t.setDaemon(true)
    t.start()

def on_sceneadded(scenename):
    scene = tundra.Scene().GetSceneRaw(scenename)
    throw_ent = scene.CreateEntityLocalRaw([]);
    throw_ent.SetName("Throwing");
    throw_ent.SetTemporary(True);
    throw_ent.Action("CatchObject").connect("Triggered(QString, QString, QString, QStringList)", on_catch)
    throw_ent.Action("ThrowObject").connect("Triggered(QString, QString, QString, QStringList)", on_throw)
    tundra.Frame().DelayedExecute(.5).connect(
        'Triggered(float)', lambda t: throw_ent.Exec(1, "ThrowObject", "Fish", scenename))

def get_context_others(cxname, callback):
    h, p, c = cxserver_hostname, cxserver_port, cgi.escape(cxname)
    qt_http_get("http://%s:%s/members/%s" % (h, p, c), callback)
    
def throw_to(who, msg):
    msg_string = json.dumps(msg);
    tcp_send_thread(msg_string, who, 4242)


def on_throw(entname, scenename):
    scene = tundra.Scene().GetSceneRaw(scenename)
    if not scene:
        tundra.Console().LogError("throw: scene %s not found" % repr(scenename))
    e = scene.GetEntityByNameRaw(entname)
    if not e:
	tundra.Console().LogError("OnThrow: entity %s not found" % repr(entname))
	return

    msg = {
	"meshref": e.mesh.meshRef,
	"meshMaterials": e.mesh.meshMaterial,
    }

    def on_context_response(data):
        who = json.loads(data)
        for guy_id, guy_info in who.items():
            print "throwing to", guy
            if guy_id != my_id():
                throw_to(guy_info, msg)

    tundra.Console().LogInfo("throw success")


    who = get_context_others("throwdemo", on_context_response)

def my_id():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('8.8.8.8', 53))
    return s.getsockname()[0]

def on_catch(msg):
    print("OnCatch called")


tundra.Scene().connect("SceneAdded(QString)", on_sceneadded)

from PythonQt.QtNetwork import QNetworkRequest, QNetworkAccessManager
from PythonQt.QtCore import QUrl
qnam = QNetworkAccessManager()

def qt_http_get(url, callback):
    req = QNetworkRequest(QUrl(url))
    def on_finish(reply):
        if reply.error():
            print "qnam http reply error"
            return
        else:
            data = reply.readAll().data()
            print 'qnam got:', repr(data)
            callback(data)

    qnam.connect("finished(QNetworkReply*)", on_finish)
    qnam.get(req)
