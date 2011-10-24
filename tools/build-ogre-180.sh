#!/bin/bash
set -e
set -x

# script to build naali and most deps.
#
# if you want to use caelum, install ogre and nvidia cg from
# ppa:andrewfenn/ogredev, change the caelum setting to 1 in
# top-level CMakeBuildConfig.txt and enable Cg module in bin/plugins-unix.cfg


viewer=$(dirname $(readlink -f $0))/..
deps=$viewer/../naali-deps
mkdir -p $deps
deps=$(cd $deps && pwd)
viewer=$(cd $viewer && pwd)

viewerdeps_svn=http://realxtend-naali-deps.googlecode.com/svn/
prefix=$deps/install
build=$deps/build
tarballs=$deps/tarballs
tags=$deps/tags

# -j<n> param for make, for how many processes to run concurrently

nprocs=`grep -c "^processor" /proc/cpuinfo` 

mkdir -p $tarballs $build $prefix/{lib,share,etc,include} $tags

export PATH=$prefix/bin:$PATH
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export LDFLAGS="-L$prefix/lib -Wl,-rpath -Wl,$prefix/lib"
export LIBRARY_PATH=$prefix/lib
export C_INCLUDE_PATH=$prefix/include
export CPLUS_INCLUDE_PATH=$prefix/include
export CC="ccache gcc"
export CXX="ccache g++"
export CCACHE_DIR=$deps/ccache

if lsb_release -c | egrep -q "lucid|maverick"; then
        which aptitude > /dev/null 2>&1 || sudo apt-get install aptitude
	sudo aptitude -y install mercurial
fi

cd $build
if [ -d ogre ]; then
	echo "Ogre build dir already exists. Skipping version control cloning."
else
	hg clone http://bitbucket.org/sinbad/ogre/ -u v1-8
fi
cd ogre
cat CMakeLists.txt |sed "s/cmake_minimum_required(VERSION 2.6.2)/set(CMAKE_INSTALL_PREFIX \/usr)\n\ncmake_minimum_required(VERSION 2.6.2)/" >t.txt
rm CMakeLists.txt
mv t.txt CMakeLists.txt
mkdir -p ogre-build
cd ogre-build
cmake ..
make -j $nprocs VERBOSE=1
sudo make install

