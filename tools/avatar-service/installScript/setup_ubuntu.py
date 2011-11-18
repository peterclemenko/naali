#!/usr/local/bin/python

## TODO: sudo passwd stuff
## meanwhile: RUN AS SUDO "sudo python setup.py"
## default php file upload size is 2M, add function to mod this value (in /etc/php5/apache2/php.ini, line: "upload_max_filesize = 2M")
##

import os
import sys
import subprocess
import shutil
import getpass
from optparse import OptionParser
import pexpect
import fileinput

###
user = os.getenv("SUDO_USER")
userhome = "/home/" + str(user)
rex = userhome + "/src/realxtend/" # realXtend install directory
rexBinDir = rex + "/naali/bin/" # 

#avatar service sources
GIT_SOURCE = "git://github.com/Lehtiz/Avatar-service.git"
# default apache webroot
WEB_ROOT = "/var/www/"
AVATAR_ROOT = WEB_ROOT + "avatar-service/"

MYSQL_HOST = "localhost"
#mysql root details set during install
MYSQL_ROOT = "root"
MYSQL_ROOT_PW = "N73J" # MYSQL root password

DATABASE_FILE = "avatardb.sql"
DB_NAME = "avatar"

###
# if useRoot is set to true, mysql root account will be used for database connections
# False creates a new user for the database, with the details below
useRoot = False # False / True

MYSQL_USER = "avatarservice"
MYSQL_USER_PW = "avatarpw123"
###


def main():
    if os.getenv("USER") == "root": #if ran with root
        # Installs apache2, php5, mysql and configures mysql root login details
        # in addition installs git and debconf-utils for a fully automated installation
        installPrograms()
        
        # Fetches avatar-service files from github and imports the database 
        setupAvatarService()
        
        if not useRoot:
            createMysqlUser(MYSQL_HOST, MYSQL_USER, MYSQL_USER_PW)
            updateMysqlConfig(MYSQL_HOST, MYSQL_USER, MYSQL_USER_PW)
        else:
            updateMysqlConfig(MYSQL_HOST, MYSQL_ROOT, MYSQL_ROOT_PW)
            
    else:
        print "Script needs to be run with sudo (apt-get install)"


def installPrograms():

    #make sure apt lists are up to date
    subprocess.call("sudo apt-get update", shell=True)

    #preq for mysql installation parameters, git sources, nodejs for glge build
    subprocess.call("sudo apt-get -y install debconf-utils git nodejs python-eventlet", shell=True)
    
    #apache2, php
    subprocess.call("sudo apt-get -y install apache2 php5-mysql libapache2-mod-php5", shell=True)
    
    #MYSQL
    #create preseed file
    preseedFile = "mysql.preseed"
    
    #make sure file does not exist, remove if it does
    if os.path.isfile(preseedFile):
        os.remove(preseedFile)
        
    with open(preseedFile, 'a') as pre:
        pre.write("mysql-server-5.1 mysql-server/" + "root_password password " + MYSQL_ROOT_PW + "\n")
        pre.write("mysql-server-5.1 mysql-server/" + "root_password_again password " + MYSQL_ROOT_PW + "\n")
        pre.write("mysql-server-5.1 mysql-server/" + "start_on_boot boolean true")
    
    #install mysql using variables from preseed
    #pipe vars from file and set
    subprocess.call("cat " + preseedFile + " | sudo debconf-set-selections", shell=True)
    subprocess.call("sudo apt-get -y install mysql-server", shell=True)
    
    #cleanup tmp files
    cleanUp(preseedFile)
        
    #restart apache server
    subprocess.call("sudo /etc/init.d/apache2 restart", shell=True)


def setupAvatarService():
    #presume tundra build already (TODO: add tundra build here?)

    #check folder dest and move to webroot/backup/ if exists
    backupDir = WEB_ROOT + "backup/"
    if os.path.exists(AVATAR_ROOT):
        if os.path.exists(backupDir):
            shutil.rmtree(backupDir)
        shutil.move(AVATAR_ROOT, backupDir)
        
    #get sources from github
    subprocess.call("git clone " + GIT_SOURCE + " " + AVATAR_ROOT, shell=True)
    
    #setup avatar-service database, import from a file
    dbRootImportFromFile(AVATAR_ROOT + DATABASE_FILE)
    
    #enable writing to models folder for adding avatar models via moderation page
    avatarServiceModelFolderRights()
    
    #setup glge for rendering in browser
    setupGlge()
    #getJquery() #added jquery to repo
    #ownForUser(AVATAR_ROOT)
    enableWebSocket() #remove if rexbuild implemented


# Creates a custom user for the mysql server with the info configured above
def createMysqlUser(host, user, pw):
    createUserTmpFile = "dbuser.sql"
    if os.path.isfile(createUserTmpFile):
        os.remove(createUserTmpFile)
    with open(createUserTmpFile, 'a') as f:
        f.write("GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, INDEX, ALTER, CREATE TEMPORARY TABLES, LOCK TABLES ON " + DB_NAME + ".* TO '" + user + "'@'" + host + "' IDENTIFIED BY '" + pw + "'")
    dbRootImportFromFile(createUserTmpFile)
    cleanUp(createUserTmpFile)

def dbRootImportFromFile(inputfile):
    subprocess.call("mysql -h" + MYSQL_HOST + " -u" + MYSQL_ROOT +" -p" + MYSQL_ROOT_PW + " < " + inputfile, shell=True)


# automatically updates action/dbconnect.php with the mysql user and password info provided above
def updateMysqlConfig(host, user, pw):
    dbConfigFileIn = "dbconnect.php"
    dbConfigFileOut = "tmp"
    
    identhost="<<HOST_NOT_SET>>"
    identuser="<<USER_NOT_SET>>"
    identpw="<<PW_NOT_SET>>"
    
    os.chdir(AVATAR_ROOT + "action/")
    
    with open(dbConfigFileIn, 'r') as feed:
        for line in feed:
            with open(dbConfigFileOut, 'a') as out:
                if identhost in line:
                    out.write(line.replace(identhost, host),)
                elif identuser in line:
                    out.write(line.replace(identuser, user),)
                elif identpw in line:
                    out.write(line.replace(identpw, pw),)
                else:
                    out.write(line)
    #replace base file with the configured dbconnect
    cleanUp(dbConfigFileIn)
    os.rename(dbConfigFileOut, dbConfigFileIn)


#get glge, build
def setupGlge():
    GLGE_SOURCE = "git://github.com/supereggbert/GLGE.git"
    GLGE_DIR = "glge/"
    os.chdir(AVATAR_ROOT)
    subprocess.call("git clone " + GLGE_SOURCE + " " + GLGE_DIR, shell=True)
    os.chdir(AVATAR_ROOT + GLGE_DIR)
    #submodules seems to need a github key to have been set at the computer so omitting these; docs and minifying
    #subprocess.call("git submodule init", shell=True)
    #subprocess.call("git submodule update", shell=True)
    #build
    subprocess.call("./build.js --without-documents", shell=True) #--without-documents


def getJquery():
    import urllib
    filename = "jquery-1.6.1.js"
    url = "http://code.jquery.com/" + filename
    os.chdir(AVATAR_ROOT + "js/")
    source = urllib.urlopen(url).read()
    with open(filename, 'w') as file:
        file.write(source)


def enableWebSocket():
    path = rexBinDir + "pymodules/"
    if os.path.isdir(path):
        port = "9999"
        filename = "websocket.ini"
        file = path + filename
        with open(file, 'w') as f:
            f.write("[websocketserver.NaaliWebsocketServer]\n")
            f.write("port=" + port)
        ownForUser(file)
    else:
        print "Realxtend Tundra does not appear to be installed,\nplease build it and run this script again"


def ownForUser(target):
    subprocess.call("chown -R "+ user + ":" + user + " " + target, shell=True)


def cleanUp(file):
    if os.path.isfile(file):
        os.remove(file)

def avatarServiceModelFolderRights():
    #folder1 = AVATAR_ROOT + "models/"
    #webuser = "www-data"
    #subprocess.call("chown -R " + webuser + ":" + webuser + " " + folder1, shell=True)
    os.chdir(AVATAR_ROOT + "scene/")
    subprocess.call("chmod 757 models/", shell=True)


"""
def buildTundra():
    if not os.path.isdir(rex):
        os.mkdir(rex)
    os.chdir(rex)
    subprocess.call("git clone -b tundra git://github.com/realXtend/naali.git", shell=True)
    enableWebSocket()
    #setupOgre()
    os.chdir(rex + "naali/tools/")
    subprocess.call("bash build-ubuntu-deps.bash", shell=True)


def setupOgre():
    #custom ogre
    ogre = "/home/" + user + "/src/ogre/"
    os.chdir(ogre)
    #get ogre
    subprocess.call("sudo apt-get install mercurial", shell=True)
    subprocess.call("hg clone http://bitbucket.org/sinbad/ogre/ -u v1-7-3", shell=True)
    #build ogre
"""

if __name__ == "__main__":
    """
    parser = OptionParser()
    parser.add_option("-p", "--password", dest="sudopw")
    (options, args) = parser.parse_args()
    if options.sudopw:
        sudopw = options.sudopw
        passwordSet=True
    """
    main()
