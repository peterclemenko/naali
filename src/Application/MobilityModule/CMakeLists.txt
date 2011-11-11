macro(use_package_qtmobility)
        message (STATUS "** Configuring QtMobility")
        
        execute_process(COMMAND qmake -query QMAKE_MKSPECS 
                        OUTPUT_VARIABLE QT4_MKSPECS)
        
        # Use the mobility.prf for determining QtMobility installation location
        # \todo Verify this works in Windows
        if(EXISTS "${QT4_MKSPECS}/features/mobility.prf")
        
            message (STATUS "-- Using mkspecs file")
            message (STATUS "       " ${QT4_MKSPECS}/features/mobility.prf)
        
            file (READ ${QT_MKSPECS_DIR}/features/mobility.prf MOBILITY_CONFIG_FILE)
            
            string( REGEX MATCH "MOBILITY_PREFIX=([^\n]+)" QTMOBILITY_PREFIX "${MOBILITY_CONFIG_FILE}")
            set (QTMOBILITY_PREFIX ${CMAKE_MATCH_1})
            
            string( REGEX MATCH "MOBILITY_INCLUDE=([^\n]+)" QTMOBILITY_INCLUDE "${MOBILITY_CONFIG_FILE}")
            set (QTMOBILITY_INCLUDE ${CMAKE_MATCH_1})
            
            string( REGEX MATCH "MOBILITY_LIB=([^\n]+)" QTMOBILITY_LIBRARY "${MOBILITY_CONFIG_FILE}")
            set (QTMOBILITY_LIBRARY ${CMAKE_MATCH_1})
            
            message (STATUS "-- Include Directories:")
            
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtPublishSubscribe)
            include_directories(${QTMOBILITY_INCLUDE}/QtPublishSubscribe)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtLocation)
            include_directories(${QTMOBILITY_INCLUDE}/QtLocation)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtServiceFramework)
            include_directories(${QTMOBILITY_INCLUDE}/QtServiceFramework)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtSystemInfo)
            include_directories(${QTMOBILITY_INCLUDE}/QtSystemInfo)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtMultimediaKit)
            include_directories(${QTMOBILITY_INCLUDE}/QtMultimediaKit)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtContacts)
            include_directories(${QTMOBILITY_INCLUDE}/QtContacts)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtVersit)
            include_directories(${QTMOBILITY_INCLUDE}/QtVersit)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtSensors)
            include_directories(${QTMOBILITY_INCLUDE}/QtSensors)
            message (STATUS "       " ${QTMOBILITY_INCLUDE}/QtMobility)
            include_directories(${QTMOBILITY_INCLUDE}/QtMobility)
            
            message (STATUS "-- Library Directories:")
            
            message (STATUS "       " ${QTMOBILITY_LIBRARY})
            link_directories(${QTMOBILITY_LIBRARY})
            
            message (STATUS "")
            
        else ()
            
            # mkspecs file for QtMobility not found, halt.
            message (FATAL_ERROR "!! Unable to locate QtMobility mkspecs file")
            
        endif ()
endmacro()

macro(link_package_qtmobility)
    target_link_libraries(${TARGET_NAME} optimized QtPublishSubscribe optimized QtLocation
                          optimized QtServiceFramework optimized QtSystemInfo optimized QtMultimediaKit
                          optimized QtContacts optimized QtVersit optimized QtSensors)
endmacro()

# Define target name and output directory
init_target (MobilityModule OUTPUT modules/core)

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES MobilityModule.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_core_modules (Framework)
use_package_qtmobility()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_package_qtmobility()
link_modules (Framework)

SetupCompileFlagsWithPCH()

final_target ()
