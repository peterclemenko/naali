# Python module for searching and playing songs from Grooveshark
# Currently bypasses Tundra Sound API and uses GStreamer for playback
# NOTE: Requires Grooveshark public API access to work
#
# Copyright (c) 2012 CIE / University of Oulu, All Rights Reserved
# For conditions of distribution and use, see copyright notice in license.txt

# \todo Allow changing song during PLAYING state
# \todo Move ServiceFusionPlayer and GStreamPlayer to a separate file

import tundra as tundra
import hashlib
import hmac
import urllib2
import json

sys.path.append('/usr/lib/python2.7/dist-packages/')
import pygst
pygst.require("0.10")
import gst

class ServiceFusionPlayer:
    def __init__(self):
        tundra.LogInfo("** Starting ServiceFusionPlayer **");
        self.g_handler = GroovesharkHandler()
        self.player = GStreamPlayer(self.PlayerCallback)

        self.state = "STOPPED"
        self.current_stream = {}
        self.time_played = 0

        assert self.g_handler.IsInitialized()
        assert tundra.Scene().connect("SceneAdded(QString)", self.SceneAdded)
        assert tundra.Frame().connect("Updated(float)", self.FrameUpdated)

        # Store previous attribute states becouse there's no way of checking which attribute actually changed
        self.prev_song_name = ""
        self.prev_state = ""

    def PlayerCallback(self, message):
        if(message == "FINISHED"):
            self.g_handler.MarkStreamFinished(self.current_stream["StreamKey"])

        entity = self.scene.GetEntityByNameRaw("radio_1")
        component = entity.dynamiccomponent

        if(not component.GetAttribute("state")):
            component.CreateAttribute("string", "state")

        if(component.GetAttribute("state") != message):
            component.SetAttribute("state", message)
        self.state = message

    def SetState(self, state):
        print("ServiceFusionPlayer: Setting state to: {0}".format(state))
        if(state == "PLAYING"):
            self.player.Play()
        elif(state == "STOPPED"):
            self.player.Stop()
        elif(state == "PAUSED"):
            self.player.Pause()

    def SceneAdded(self, name):
        self.scene = tundra.Scene().GetSceneRaw(name)
        assert self.scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", self.OnAttributeChanged)

    def FrameUpdated(self, frametime):
        if(len(self.current_stream) >= 0 and self.state == "PLAYING"):
            self.g_handler.AddStreamTime(self.current_stream["StreamKey"], frametime)

    def OnAttributeChanged(self, component, attribute, changeType):
        entity = component.ParentEntity()
        if(entity.name != "radio_1"):
            return

        if(component.typeName != "EC_DynamicComponent"):
            return

        state = component.GetAttribute("state")
        if(state != self.prev_state):
            self.prev_state = state
            if(state and state != "" and state != self.state):
                self.SetState(state)
                return

        song_name = component.GetAttribute("song")
        if(song_name != self.prev_song_name):
            self.prev_song_name = song_name
            tundra.LogInfo("ServiceFusionPlayer: Searching for song: {0}".format(song_name))

            song_id = self.g_handler.SearchSong(song_name, 1)[0]["SongID"]
            if(song_id != 0):
                print("ServiceFusionPlayer: Song found! Acquiring stream for first result..")
                self.current_stream = self.g_handler.GetStreamServer(song_id)
                if(len(self.current_stream["url"]) > 0):
                    print("ServiceFusionPlayer: ..Success!")
                    self.player.SetStreamURI(self.current_stream["url"])
                else:
                    tundra.LogInfo("ServiceFusionPlayer: Couldn't acquire stream url!")
            else:
                tundra.LogInfo("ServiceFusionPlayer: No song found with query \"{0}\"".format(song_name))

# Handles Grooveshark session, searching, stream fetching and stream handling.
class GroovesharkHandler:
    def __init__(self):
        self.key = ""
        self.secret = ""

        self.api_url = "https://api.grooveshark.com/ws3.php?sig="
        self.session_id = ""
        self.active_streams = {}

        assert self.key != ""
        assert self.secret != ""

        assert self.StartSession()

        self.country = self.GetCountry()

        self.initialized = True

    def IsInitialized(self):
        return self.initialized

    def Signature(self, data):
        sig = hmac.new(self.secret, data)
        return sig.hexdigest()

    def Request(self, method, params={}):
        data = {}
        data["method"] = method
        data["parameters"] = params
        data["header"] = {}
        data["header"]["wsKey"] = self.key
        if(method != "startSession"):
            data["header"]["sessionID"] = self.session_id

        payload = json.dumps(data)
        sig = self.Signature(payload)

        #tundra.LogInfo("<request>{0}</request>".format(payload))

        req = urllib2.Request(self.api_url+sig, payload)
        response = urllib2.urlopen(req).read()

        #tundra.LogInfo("<response>{0}</response>".format(response))

        return json.loads(response)

    def StartSession(self):
        response = self.Request("startSession")
        if response["result"]["success"] == True:
            self.session_id = response["result"]["sessionID"]
            tundra.LogInfo("GroovesharkHandler: Session started succesfully!")
            return True
        else:
            return False

    def GetCountry(self):
        response = self.Request("getCountry")
        return response["result"]

    def SearchSong(self, song_name, limit):
        method = "getSongSearchResults"
        params = {}
        params["query"] = song_name
        params["country"] = self.country
        params["limit"] = limit

        response = self.Request(method, params)

        if(len(response["result"]["songs"]) > 0):
            return response["result"]["songs"]
        else:
            return []

    def GetStreamServer(self, song_id, low_bitrate = False):
        method = "getStreamKeyStreamServer"
        params = {}
        params["songID"] = song_id
        params["country"] = self.country
        params["lowBitrate"] = low_bitrate

        response = self.Request(method, params)

        result = response["result"]
        if(len(result) > 0):
            self.active_streams[result["StreamKey"]] = {}
            self.active_streams[result["StreamKey"]]["SongID"] = song_id
            self.active_streams[result["StreamKey"]]["ServerID"] = result["StreamServerID"]
            self.active_streams[result["StreamKey"]]["TimeElapsed"] = 0
            self.active_streams[result["StreamKey"]]["Acked"] = False
            return response["result"]
        else:
            return []

    def AddStreamTime(self, stream_key, time_elapsed):
        if(self.active_streams[stream_key]["Acked"] == True):
            return

        self.active_streams[stream_key]["TimeElapsed"] = self.active_streams[stream_key]["TimeElapsed"] + time_elapsed
        if(self.active_streams[stream_key]["TimeElapsed"] / 30 >= 1):
            self.__MarkStreamOver30s(stream_key, self.active_streams[stream_key])

    def MarkStreamFinished(self, stream_key):
        print("GroovesharkHandler: Marking stream finished")
        method = "markSongComplete"
        params = {}
        params["songID"] = self.active_streams["stream_key"]["SongID"]
        params["streamKey"] = stream_key
        params["streamServerID"] = self.active_streams["stream_key"]["ServerID"]

        response = self.Request(method, params)

        del self.active_streams["stream_key"]

    def __MarkStreamOver30s(self, key, stream):
        print("GroovesharkHandler: Marking stream duration >30s")
        method = "markStreamKeyOver30Secs"
        params = {}
        params["streamKey"] = key
        params["streamServerID"] = stream["ServerID"]

        response = self.Request(method, params)
        stream["Acked"] = True


# Simple GStreamer player for now
class GStreamPlayer:
    def __init__(self, callback):
        self.callback = callback
        self.player = gst.element_factory_make("playbin", "player")
        self.uri = ""

        bus = self.player.get_bus()
        bus.add_signal_watch()
        bus.connect("message", self.OnMessage)

    def OnMessage(self, bus, message):
        if(message.type == gst.MESSAGE_EOS):
            self.player.set_state(gst.STATE_NULL)
            self.uri = ""
            self.callback("FINISHED")
        elif(message.type == gst.MESSAGE_ERROR):
            self.player_set_state(gst.STATE_NULL)
            self.uri = ""
            self.callback("ERROR")

    def SetStreamURI(self, uri):
        self.uri = uri
        self.player.set_property("uri", self.uri)

    def Play(self):
        if(self.uri != ""):
            self.player.set_state(gst.STATE_PLAYING)
            self.callback("PLAYING")
        else:
            self.Stop()

    def Pause(self):
        if(self.uri != ""):
            self.player.set_state(gst.STATE_PAUSED)
            self.callback("PAUSED")

    def Stop(self):
        self.player.set_state(gst.STATE_NULL)
        self.uri = ""
        self.callback("STOPPED")

if __name__ == "__main__":
    r = ServiceFusionPlayer()
