import BaseHTTPServer, SocketServer, threading
import sys, os
import urllib, urllib2
import subprocess, gzip

from PIL import Image
import getopt
import mesh_transform


class Handler(BaseHTTPServer.BaseHTTPRequestHandler):

    def logMessage(self, message):
        print message

    def parseURL(self, URL):
        """ self.parseURL(URL): This method will split the incoming URL into parameters
            which will be used later in the proxy code. After calling this method, internal
            class params:
                - URL
                - asset
                - params (LOD, profile)
            are filled and can be used accordingly afterwards.

            Note: This method is only a parser. It will recognize the params it is programmed to do
            and it will make sanity checking on those. Apart from that it will not use the params for
            anything. It would be the duty of following code handling the actual asset.
        """
        self.logMessage("Incoming URL: "+str(URL))
        try:
            self.baseurl, self.asset = URL.split("?", 1)
            try: self.asset, self.params = self.asset.split("&", 1)
            except ValueError: self.params = None
        except ValueError: self.asset = None
        try:
            if self.asset[-1] == "/": self.asset = self.asset[:-1] # Remove trailing slash
        except TypeError:
            pass

        self.logMessage("URL: "+self.baseurl)
        self.logMessage("asset: "+str(self.asset))
        self.logMessage("params: "+str(self.params))
        #self.logMessage("current thread: "+str(threading.current_thread()))

        if self.asset == None or self.params == None:
            return

        for i in self.params.split("&"):
            self.logMessage("Processing param: "+str(i))
            try:
                p, v = i.split("=")
                try: v = int(v) # Force translation into integer
                except ValueError: self.logMessage("Param decoding failed. Ignoring!"); continue
            except ValueError: self.logMessage("Param decoding failed. Ignoring!"); continue

            if p.lower() == "lod":
                self.p_LOD = v
                self.logMessage("Detected param "+str(p)+" with value "+str(v))
            elif p.lower() == "profile":
                self.p_Profile = v
                self.logMessage("Detected param "+str(p)+" with value "+str(v))
            else:
                self.logMessage("Errorneous param "+str(p)+". Ignoring!")

    def pushData(self, localfile, mimetype, originalurl, suffix, newsuffix, headers, contentencoding=None):
        """ pushData(localfile, mimetype):
            - This method finalizes the asset transfer, once the asset has been manipulated first.
            - It formulates the HTTP response, fills in the header according to mimetype, and finally
              pushes the data from a file which is given as a parameter. File is treated as binary
              stream and is pushed as is.
        """
        import hashlib, base64
        from time import gmtime, strftime
        try: f = open(localfile)
        except IOError:
            self.send_response(404)
            self.logMessage("pushData() file open failed! Sending 404")
            return
        data = f.read() # Fixme: very big files will cause a jam
        f.close()
        html_time = list(gmtime())
        if html_time[3] < 23:
            html_time[3] += 1 # Add one hour to current time (cache item expires in one hour)
                              # Fixme: This is a bad way of checking time overflow.. :|
        md5 = hashlib.md5()
        md5.update(data)
        self.send_response(200)
        self.send_header("Content-Type", mimetype)
        self.send_header("Content-Length", os.path.getsize(localfile))
        self.send_header("Expires", strftime("%a, %d %b %Y %H:%M:%S +0000", html_time))
        self.send_header("Cache-Control", "max-time=3600,public") # HTML/1.1 cache control for one hour
        self.send_header("Content-MD5", base64.b64encode(md5.digest()))
        if contentencoding != None:
            self.send_header("Content-Encoding", contentencoding)
        self.end_headers()
        self.wfile.write(data)
        os.unlink(localfile)
        self.logMessage("data push successful with mimetype: "+str(mimetype))

###############################################################################
# Image manipulation code
#

    def handleImageAndResponse_Profile1(self, originalurl, localfile, mimetype, suffix, headers):
        """ This is the profile which translates all images into ETC1 and uses bzip2 to compress them
        """
        self.logMessage("LOD=%d, profile=%d, scaling image" % (self.p_LOD, self.p_Profile))
        if self.p_LOD == 1:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+".ppm", "-f", "ppm", "-p", "6"])
        if self.p_LOD == 2:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+".ppm", "-f", "ppm", "-p", "13"])
        if self.p_LOD == 3:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+".ppm", "-f", "ppm", "-p", "25"])
        if self.p_LOD == 4:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+".ppm", "-f", "ppm", "-p", "50"])
        if self.p_LOD == 5:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+".ppm", "-f", "ppm", "-p", "100"])
        self.logMessage("LOD=%d, packing to ETC1" % self.p_LOD)
        subprocess.call(["./etcpack/etcpack", str(localfile)+".ppm", str(localfile)+".pkm"])
        os.unlink(str(localfile)+".ppm")
        self.logMessage("LOD=%d, gzip" % self.p_LOD)
        gf = gzip.GzipFile(str(localfile)+".pkm.gz", "wb")
        f = open(str(localfile)+".pkm")
        gf.write(f.read())
        f.close()
        gf.close()
        os.unlink(str(localfile)+".pkm")
        self.logMessage("done compression")
        self.pushData(str(localfile)+".pkm.gz", "image/x-etc1", originalurl, suffix, "pkm.gz", headers, "gzip")

    def handleImageAndResponse_Profile2(self, originalurl, localfile, mimetype, suffix, headers):
        """ This is the profile which scales down all images and keeps original format
        """
        self.logMessage("LOD=%d, profile=%d, scaling image" % (self.p_LOD, self.p_Profile))
        if self.p_LOD == 1:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+"."+suffix, "-f", mimetype, "-p", "6" ])
        if self.p_LOD == 2:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+"."+suffix, "-f", mimetype, "-p", "13"])
        if self.p_LOD == 3:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+"."+suffix, "-f", mimetype, "-p", "25"])
        if self.p_LOD == 4:
            subprocess.call(["python", "img_transform.py", "-i", str(localfile), "-o", str(localfile)+"."+suffix, "-f", mimetype, "-p", "50"])
        if self.p_LOD == 5:
            self.pushData(str(localfile), "image/"+mimetype, originalurl, suffix, suffix, headers)
            return
        self.pushData(str(localfile)+"."+suffix, "image/"+mimetype, originalurl, suffix, suffix, headers, None)

    def handleImageAndResponse(self, originalurl, localfile, mimetype, suffix, headers):
        """ handleImageAndResponse(localfile, imagetye):
            - This method will be called once the incoming asset is recognized to be one of
              the supported image formats.
            - The method will do whatever for the image file, based on the given HTTP params.
            - Once the file manipulation has been completed, it will be saved into temporary
              directory, and sent to requester via pushData() method.
        """
        try: image = Image.open(localfile)
        except IOError:
            self.send_response(404)
            self.logMessage("PIL image loading failed")
            return
        if self.p_Profile == 1: # scale + ETC1 + gzip
            self.handleImageAndResponse_Profile1(originalurl, localfile, mimetype, suffix, headers)
            return
        else: # Scaling only, format kept (this is also default, if profile is not defined)
            self.handleImageAndResponse_Profile2(originalurl, localfile, mimetype, suffix, headers)
            return

###############################################################################
# Mesh manipulation code
#

    def handleMeshAndResponse_Profile1(self, originalurl, localfile, mimetype, suffix, headers):
        """ handleMeshAndReponse_Profile1(): This is the mesh detail reduction filter which
            drops the vertex and face detail in percentage steps
        """
        if self.p_LOD < 5: # LODs 1-4 require adjustments:

            if self.p_LOD == 1: mlx_file = "quad20.mlx"
            if self.p_LOD == 2: mlx_file = "quad35.mlx"
            if self.p_LOD == 3: mlx_file = "quad50.mlx"
            if self.p_LOD == 4: mlx_file = "quad70.mlx"

            try:
                # ogre mesh -> ogre mesh XML
                rc = subprocess.call(["OgreXMLConverter", localfile, localfile+".xml"])
                if rc != 0: raise ValueError

                # Ogre mesh XML -> obj
                self.logMessage("Trying assimp")
                rc = subprocess.call(["assimp", "export", localfile+".xml", localfile+".obj"])
                if rc != 0: raise ValueError
                os.unlink(localfile+".xml")

                # obj -> Collada + optimize
                self.logMessage("Trying meshlabserver")
                rc = subprocess.call(["xvfb-run", "-a", "./simplify/meshlabserver", "-i", localfile+".obj", "-o", localfile+".dae", "-s", mlx_file, "-om", "vn", "wt"])
                if rc != 0: raise ValueError
                os.unlink(localfile+".obj")

                # Collada -> Ogre Mesh
                self.logMessage("Trying OgreAssimpconverter")
                rc = subprocess.call(["OgreAssimpConverter", localfile+".dae"])
                if rc != 0: raise ValueError
                os.unlink(localfile+".dae")

                # Ogre Mesh -> Ogre Mesh XML
                self.logMessage("Trying OgreXMLconverter")
                rc = subprocess.call(["OgreXMLConverter", localfile+"_dae.mesh", localfile+".xml"])
                if rc != 0: raise ValueError
                os.unlink(localfile+"_dae.mesh")

            except OSError:
                self.logMessage("OSerror during the mesh process. check tool installation. Abort")
                self.pushData(localfile, "model/mesh", originalurl, "mesh", "mesh", headers, None)
                return
            except ValueError:
                self.logMessage("ValueError during the mesh process. Abort")
                self.pushData(localfile, "model/mesh", originalurl, "mesh", "mesh", headers, None)
                return


            # At this point we have converted .xml file waiting. Compress and send it as such

            self.logMessage("LOD=%d, packing to .xml.gz" % self.p_LOD)
            gf = gzip.GzipFile(str(localfile)+".xml.gz", "wb")
            f = open(str(localfile)+".xml")
            gf.write(f.read())
            f.close()
            gf.close()
            os.unlink(str(localfile)+".xml")
            self.logMessage("done compression")
            self.pushData(str(localfile)+".xml.gz", "model/x-ogremesh", originalurl, suffix, "xml.gz", headers, "gzip")

        else: # LOD = 5
            print "Pushing unmodified mesh LOD=5"
            self.pushData(localfile, "model/mesh", originalurl, "mesh", "mesh", headers, None)

    def handleMeshAndResponse(self, originalurl, localfile, mimetype, suffix, headers):
        """ handleMeshAndReponse(localfile, meshtype):
            - TBD
        """
        # For meshes, only one profile is implemented. Hence, we do not (yet) check for the profile value
        # instead execute the profile 1 implementation
        self.handleMeshAndResponse_Profile1(originalurl, localfile, mimetype, suffix, headers)

###############################################################################
# HTTP GET request handler
#

    def do_GET(self):
        """ do_GET(): custom handler for HTTP GET method
            In the proxy, it is assumed for the translation that the
            incoming asset URL will come in following format:

            http://proxyname/?originalassetserver/path/to/asset.png&param1=X&param2=Y
            |----baseurl----||---------------asset----------------||-----params-----|

            Which is being decoded into baseurl, asset reference and params, as shown.

            Supported parameters are:

            - LOD: A number ranging from 1-5 stating the dynamic LOD level which must be
              passed to client. Default LOD is 1, meaning the lower possible translation quality
            - Profile: A ID of the profile

            Note: Squid URLManger passes all relevant URLs to this script. It will handle the
            decision, which URLs shall be translated. All traffic which reaches this point of
            process, shall be translated.
        """
        self.baseurl = ""
        self.asset = ""
        self.params = ""
        self.p_LOD = 5              # Defaults are LOD=5 (prefer no change in the image contents)
        self.p_Profile = 2          # and Profile = 2 (keep original image format and scale only if needed)

        self.logMessage("---")
        self.logMessage("Incoming URL: "+self.path)
        self.parseURL(self.path)

        if self.asset == None:
            self.send_response(404)
            self.logMessage("Given asset is None, abort with 404")
            return

        self.logMessage("Retrieving remote file: "+str(self.asset))
        local_filename, headers = urllib.urlretrieve("http://" + self.asset)
        self.logMessage("Downloaded into: "+str(local_filename))
        if os.path.exists(local_filename) == False:
            self.send_response(404)
            self.logMessage("Downloading of the asset failed")
            return

        #req = urllib2.Request("http://" + self.asset)
        #try:
        #    urlhandle = urllib2.urlopen(req)
        #except urllib2.HTTPError, e:
        #    self.logMessage("HTTPError on url=%s Code=%d" % ("http://"+self.asset, e.code))
        #    self.send_response(404)
        #    return
        #except urllib2.URLError, e:
        #    self.logMessage("URLError on url=%s, reason=%s" % ("http://"+self.asset, e.reason))
        #    self.send_response(404)
        #    return
        #data_header = urlhandle.info()
        #data = urlhandle.read()
        #urlhandle.close()
        #self.logMessage("URL read into memory, size=%d bytes" % len(data))

        #self.logMessage("URL.headers():")
        #self.logMessage(headers)

        # At the moment jpg, png, gif and tga file formats are passed
        # from squid url mangler. In this code we shall distinguish just between
        # image and mesh assets, and act accordingly.

        for t in ["jpg", "jpeg", "png", "gif", "tga"]:
            if self.asset.lower().endswith(t):
                mimetype = t
                if t == "jpg": mimetypet = "jpeg" # This is to force PIL to detect Jpeg image format
                self.logMessage("Imagetype "+str(t)+" detected.")
                self.handleImageAndResponse("http://"+self.asset, local_filename, mimetype, t, headers)
                #self.handleImageAndResponse(data_header, data, t)
                return

        # Same applies for the meshes. At the moment the code below supports only
        # Ogre meshes, but will be extended to other formats as well.

        for t in ["mesh"]:
            if self.asset.lower().endswith(t):
                self.logMessage("Meshtype "+str(t)+" detected.")
                self.handleMeshAndResponse("http://"+self.asset, local_filename, "model/mesh", t, headers)
                #self.handleMeshAndResponse(data_header, data, t)
                return


class MyHTTPServer(SocketServer.ThreadingMixIn, BaseHTTPServer.HTTPServer):
    pass

if __name__ == "__main__": # if run standalone
    try:
        opts, args = getopt.getopt(sys.argv[1:], "uh", ["urlparsertest", "httpserver"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err)
        print "Usage: httpserver.py [-u|urlparsertest] [-h|httpserver]"
        sys.exit(2)

    for o, a in opts:
        if o in ("-u", "--urlparsertest"):
            testurls = [ "http://proxyserver.fi?chiru.cie.fi/path/to/asset.png&LOD=1&Profile=2&Lala=5",
                         "http://proxyserver.fi?chiru.cie.fi/path/to/asset.tga&LOD=1&Profile=2&Lala=5&extra=2",
                         "http://proxyserver.fi?chiru.cie.fi/path/to/asset.jpg",
                         "http://proxyserver.fi?chiru.cie.fi/path/to/asset.JPEG&LOiD=1",
                         "http://proxyserver.fi?chiru.cie.fi/path/to/asset.png&pRoFiLe=56",
                         "http://proxyserver.fi?chiru.cie.fi/path/to/asset.png&LOd=2&profile=ldlld" ]
            h = Handler()
            for i in testurls:
                h.parseURL(i)
            sys.exit()
        elif o in ("-h", "--httpserver"):
            PORT = 8000
            httpd = MyHTTPServer(("", PORT), Handler)
            print "serving at port", PORT
            httpd.serve_forever()
        else:
            assert False, "unhandled option"

    print "Done."



