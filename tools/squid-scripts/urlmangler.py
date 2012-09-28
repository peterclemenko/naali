#!/usr/bin/python
#
# Input string from Squid is in format:
#
# http://localhost/world/lvm/slide1.jpg 127.0.0.1/localhost.localdomain - GET myip=127.0.0.1 myport=3128
#
# URLMangler shall be interested in the first part of the string, namely the URL. Rest of the parameters
# are optional and shall be ignored at this point of implementation.
#
# Optionally, the URL shall contain chiru specific parameters, separated by URL parameters, in this case
# the URL is formulated like this.
#
# http://localhost/world/lvm/slide1.jpg?param=5 127.0.0.1/localhost.localdomain - GET myip=127.0.0.1 myport=3128
#
# Parsing shall be forwarded into:
#  - url = complete URL with params  (e.g. http://localhost/world/lvm/slide1.jpg?param=5)
#  - head = URL stripped from params (e.g. http://localhost/world/lvm/slide1.jpg)
#
# Translation shall be done using translator_prefix, and complete URL. For example with above information
# the output shall be:
#
# http://localhost:8000/?localhost/world/lvm/slide1.jpg?param=5
#

import os, sys

translator_prefix = "http://chiru.cie.fi:8000/?"

logfilename = "/tmp/urlmangler_" + str(os.getpid()) + ".log"
f = open(logfilename, "w")

while 1:
    url_temp = sys.stdin.readline()
    if not url_temp:
        f.write("Making exit!")
        f.flush()
        f.close()
        break
    url_temp = url_temp.strip()
    f.write("Received string from stdin: " + url_temp + "\n")

    #print url_temp
    #sys.stdout.flush()
    #f.flush()
    #continue

    t_url = url_temp.split(" ", 1)
    url = t_url[0]
    #f.write("Split URL: " + str(url) + "\n")
    t_head = url.split("?", 1)
    head = t_head[0]
    if head.endswith("/"): head = head[:-1] # Remove trailing slash
    #f.write("URL Head is: "+ str(head) + "\n")

    # We shall redirect all images, and all meshes into translator. Other assets
    # will go pass through, as-is.

    rewrite_url = ""
    for i in [ ".png", ".jpg", ".jpeg", ".tga", ".gif", ".mesh" ]:
        if head.endswith(i):
            url = url.replace("?", "&")
            #f.write("Found translatable asset in the URL. Type: '"+str(i)+"'\n")
            rewrite_url = translator_prefix + url[len("http://"):len(url)]
            f.write("Translated URL: "+str(rewrite_url)+"\n")
            break

    print rewrite_url.strip()
    sys.stdout.flush()
    f.flush()

sys.exit()
