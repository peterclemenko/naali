macro(configure_pcl)
    if(NOT WIN32)
    sagase_configure_package(pcl
        NAMES pcl PCL
        COMPONENTS
        PREFIXES ${ENV_TUNDRA_DEP_PATH} "/usr")
    sagase_configure_report(pcl)
    else ()
        # Implement me
    endif()
endmacro(configure_pcl)


# Define target name and output directory
init_target (ObjectCaptureModule OUTPUT plugins)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
#file (GLOB MOC_FILES ObjectCaptureModule.h)

AddSourceFolder (MeshReconstructor)
AddSourceFolder (CloudProcessor)

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})

configure_pcl()
use_package(PCL)

use_app_modules (ObjectCaptureModule)
use_core_modules (Framework Math)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_modules (Framework)
link_package (PCL)

SetupCompileFlagsWithPCH()

final_target ()

