# Python module for searching and playing songs from Grooveshark
# Currently bypasses Tundra Sound API and uses GStreamer for playback
# NOTE: Requires Grooveshark public API access to work
#
# Copyright (c) 2012 CIE / University of Oulu, All Rights Reserved
# For conditions of distribution and use, see copyright notice in license.txt

# \todo Separate ServiceFusion integration from GroovesharkHandler implementation
# \todo Add Grooveshark stream control messages

import tundra as tundra
import hashlib
import hmac
import urllib2
import json

sys.path.append('/usr/lib/python2.7/dist-packages/')
import pygst
pygst.require("0.10")
import gst

class GroovesharkHandler:
    def __init__(self):
        tundra.LogInfo("** Starting Tundra Grooveshark handler **");

        self.key = ""
        self.secret = ""

        self.api_url = "https://api.grooveshark.com/ws3.php?sig="
        self.session_id = ""

        if(self.key == "" or self.secret == ""):
            print("GroovesharkHandler: Key or Secret missing, halting!")
            return

        assert tundra.Scene().connect("SceneAdded(QString)", self.SceneAdded)

        assert self.StartSession()

        self.country = self.GetCountry()

        self.player = GStreamPlayer()

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
        assert len(response["result"]) >= 0
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

        if(len(response["result"]) > 0):
            return response["result"]
        else:
            return []

    def SceneAdded(self, name):
        self.scene = tundra.Scene().GetSceneRaw(name)
        tundra.LogInfo("Got new scene: {0}".format(name))

        assert self.scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", self.OnAttributeChanged)

    def OnAttributeChanged(self, component, attribute, changeType):
        entity = component.ParentEntity()
        if(entity.name != "radio_1"):
            return

        if(component.typeName != "EC_DynamicComponent"):
            return

        song_name = component.GetAttribute("song")
        if(song_name == ""):
            return

        tundra.LogInfo("GroovesharkHandler: Got input song: {0}".format(song_name))

        song_id = self.SearchSong(song_name, 1)[0]["SongID"]
        if(song_id != 0):
            tundra.LogInfo("Found song with id: {0}".format(song_id))
            stream_url = self.GetStreamServer(song_id)["url"]
            if(len(stream_url) > 0):
                tundra.LogInfo("Got song stream url: {0}".format(stream_url))
                self.player.SetStreamURI(stream_url)
                self.player.Play()
            else:
                tundra.LogInfo("Couldn't acquire stream url")
        else:
            tundra.LogInfo("No song found")

# Simple GStreamer player for now
class GStreamPlayer:
    def __init__(self):
        self.player = gst.element_factory_make("playbin", "player")
        self.uri = ""

    def SetStreamURI(self, uri):
        self.uri = uri
        self.player.set_property("uri", self.uri)

    def Play(self):
        if(self.uri != ""):
            self.player.set_state(gst.STATE_PLAYING)

    def Stop(self):
        self.player.set_state(gst.STATE_READY)
        self.uri = ""

if __name__ == "__main__":
    r = GroovesharkHandler()
