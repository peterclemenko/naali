# Define target name and output directory
init_target (QMLPlugin OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES QMLPlugin.h EC_QML.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_core_modules (Framework Scene OgreRenderingModule Math)
use_app_modules (SceneWidgetComponents)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )

link_ogre()
link_modules (Framework Scene OgreRenderingModule SceneWidgetComponents)

SetupCompileFlagsWithPCH()

final_target ()
