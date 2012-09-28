"""
Coax asyncore and PythonQt to play together using
QSocketNotifier.
"""

import asyncore, json, traceback
import PythonQt, tundra
from collections import defaultdict
from PythonQt.QtCore import QSocketNotifier


import select

notifiers = defaultdict(list)

class delhook_dict(dict):
    def __setitem__(self, fd, v):
        print 'new dict item:', fd, v
        if fd in self:
            del self[fd]
        hook_fd(fd, v)
        dict.__setitem__(self, fd, v)

    def __delitem__(self, fd):
        print 'del dict item:', fd
        unhook_fd(fd)
        dict.__delitem__(self, fd)

my_socket_map = delhook_dict()

def hook_all():
    for fd, obj in my_socket_map.items():
        hook_fd(fd, obj)

def unhook_fd(fd):
    for n in notifiers[fd]:
        n.setEnabled(False)

    del notifiers[fd]

def hook_fd(fd, obj):
    handler_table = [
        (QSocketNotifier.Read, make_handler(select.POLLIN)),
        #(QSocketNotifier.Write, make_handler(select.POLLOUT)),
        (QSocketNotifier.Exception, make_handler(select.POLLERR))]

    for evtype, handler in handler_table:
        n = QSocketNotifier(fd, evtype)
        n.connect('activated(int)', handler)
        notifiers[fd].append(n)


def make_handler(flag):
    def handler(fd):
        obj = my_socket_map.get(fd)
        if obj is None:
            print "asyncore handler called for unknown socket", fd
            return
        asyncore.readwrite(obj, flag)

    return handler

import asyncore, socket


import asynchat
class LineHandler(asynchat.async_chat):
    def __init__(self, sock, map=my_socket_map):
        asynchat.async_chat.__init__(self, sock, map)
        self.set_terminator("\r\n")
        self.data = ""

    def writable(self):
        print 'writable called, vars', self.producer_fifo, self.connected
        asynchat.async_chat.writable(self)

    def collect_incoming_data(self, data):
        print 'collecting', repr(data)
        self.data = self.data + data

    def found_terminator(self):
        try:
            req = json.loads(self.data)
        except ValueError, what:
            self.report_err(str(what))
        except:
            self.report_err("json decode error")
        else:
            self.handle_request(req)
        self.data = ""
    
    def report_err(self, msg):
        self.push(json.dumps(dict(error=msg)))
        self.push("\r\n")


    def handle_request(self, req):
        op = req.get('op')
        if not op:
            self.report_err("missing op in request")
        else:
            methname = 'op_' + str(op)
            del req['op']
            meth = getattr(self, methname, None)
            if not meth:
                self.report_err("unknown op in request")
                return
            r=None
            try:
                r = meth(**req)
            except TypeError:
                self.report_err("bad args, usage: %s" % meth.__doc__)
                print "type error, traceback:"
                traceback.print_exc()
                return
            except:
                self.report_err("handler raised unhandled exception")
                print "error handling request for", op
                traceback.print_exc()
                return
            try:
                if r is None:
                    r = dict()
                self.push(json.dumps(r))
                self.push("\r\n")
            except (OverflowError, TypeError, ValueError), e:
                self.report_err("error serializing response")

    def op_action(self, exectype, scene, entity, action, params):
        "exectype: execution type bitfield, scene: scene name, entity: entity name or integer id, action: action name, params: list of strings"

        if isinstance(exectype, basestring):
            exectype = exectype_parse(exectype)
            
        if isinstance(entity, basestring):
            ent = tundra.Scene().MainCameraScene().GetEntityByNameRaw(entity)
        else:
            ent = tundra.Scene().MainCameraScene().GetEntityRaw(entity)
        if not ent:
            self.report_err("entity not found for json aftion (name/id=%s)" % entity)
            print "ent not found"
            return
        rv = ent.Exec(exectype, action, *params)
        print "action %s executed on entity %d" % (action, ent.id)
        return dict(returnvalue=rv)

def exectype_parse(s):
    execint = 0
    for w in s.split(","):
        if w.lower() == 'local':
            execint |= 1
        elif w.lower() == 'server':
            execint |= 2
        elif w.lower() == 'peers':
            execint |= 4
        else:
            print "unknown exec type", w
    return execint

class LineServer(asyncore.dispatcher):
    def __init__(self):
        asyncore.dispatcher.__init__(self, map=my_socket_map)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind(('', 4242))
        self.listen(5)
    
    def writable(self):
        return False

    def handle_accept(self):
        sock, addr = self.accept()
        print 'line client from %s (socket fd %d)' % (addr, sock.fileno())
        
        LineHandler(sock, map=my_socket_map)


if not tundra.IsServer():
#if 1:
    hook_all()
    server = LineServer()
