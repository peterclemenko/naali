#!/bin/bash
bashtrapExit()
{
	ps x | grep '\.\/Tundra --server.* scenes\/.*' | awk '{print $1}' | xargs kill
    if [ -e jsmodules/startup/$filename ]; then
        rm -f jsmodules/startup/$filename
    fi
}
bashtrapKillTerm()
{
    exitCode="$?"
    #This trap is used to clean up possible temp files, etc. if script recieves TERM KILL signal.
    echo 'Program sent exit code: ' $exitCode
    rm -f jsmodules/startup/$filename
    echo 'Killing servers now...'
	echo ps x | grep '\.\/Tundra --server.* scenes\/.*'
	ps x | grep '\.\/Tundra --server.* scenes\/.*' | awk '{print $1}' | xargs kill
    exit $?
}
# Test if file exists which is given as 1st parameter for this script.
if [ ! -e "$1" ];
then
    echo "File '"$1"' does not exists! Example startup scripts found at:" $PWD/startupScripts/
    echo '------------'
    ls -l $PWD/startupScripts/ | grep '\.js'
    echo '------------'
else
    trap bashtrapKillTerm INT TERM KILL
    trap bashtrapExit EXIT
    filename=$(basename $1)
    
    # Start two local servers.
    ./Tundra --server --file scenes/Avatar/scene.txml --protocol tcp --port 2345 &
    ./Tundra --server --file scenes/DayNight/Scene.txml --protocol tcp --port 2346 &
    
    # Copy desired javascript file to jsmodules/startup/ for viewer to execute.
    cp startupScripts/$filename jsmodules/startup/
    
    # Start viewer with valgrind tool memcheck.
    ./Tundra --storage scenes/ --fullscreen

    echo ''
    echo 'End of testdrive.'
    echo ''
fi
