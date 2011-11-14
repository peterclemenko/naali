# Define target name and output directory
init_target (EC_Menu)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES EC_MenuContainer.h EC_MenuItem.h MenuDataItem.h MenuDataModel.h MenuRendererInterface.h RingMenuRenderer.h ShelveMenuRenderer.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_package_bullet()
# Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule TundraLogicModule RexCommon PhysicsModule
use_core_modules (Framework Scene OgreRenderingModule PhysicsModule)# SceneWidgetComponents)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

#link_ogre()
#link_package_bullet()
#Core Foundation Interfaces Ui Input Scene Asset OgreRenderingModule TundraLogicModule RexCommon
link_modules (Framework Scene )

#link_entity_components (EC_Billboard)

#link_entity_components (EC_WidgetCanvas)
# PhysicsModule

SetupCompileFlagsWithPCH()

final_target ()
