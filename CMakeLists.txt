cmake_minimum_required(VERSION 3.5...4.99)
PROJECT(OST C CXX)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-fno-sized-deallocation)
add_compile_options(-fPIC)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
set(THREADS_PREFER_PTHREAD_FLAG ON)

#find_package(Qt5Core)
find_package(Git)
find_package(CFITSIO REQUIRED)

find_package(INDI REQUIRED)
#find_package(INDI COMPONENTS clientqt5 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(Nova REQUIRED)
find_package(GSL REQUIRED)
#find_package(OpenCV REQUIRED )
find_package(WCSLIB REQUIRED)
find_package(QT NAMES Qt5 COMPONENTS Gui Widgets Core Concurrent Network WebSockets Sql Scxml Xml REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Gui Widgets Core Concurrent Network WebSockets Sql Scxml Xml)
find_package(StellarSolver REQUIRED)
find_library(avahi-client-lib avahi-client REQUIRED)
find_library(avahi-common-lib avahi-common REQUIRED)
find_path(avahi-client-includes avahi-client/client.h REQUIRED)
find_path(avahi-common-includes avahi-common/defs.h REQUIRED)




include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/model)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues)
include_directories(${INDI_INCLUDE_DIR})
include_directories(${CFITSIO_INCLUDE_DIR})
include_directories(${GSL_INCLUDE_DIRS})
include_directories(${Qt5_DIR})
include_directories(${QT5Declarative_INCLUDE_LIBRARIES} )
include_directories(${QT5Declarative_INCLUDE_DIRS} )
include_directories(${STELLARSOLVER_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf)

include(CMakeCommon)

# Generate .qm files from .ts files using lrelease
set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/ost_fr.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/ost_en.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/ost_de.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/ost_es.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/ost_it.ts
)

# Find lrelease executable
find_program(QT_LRELEASE_EXECUTABLE NAMES lrelease lrelease-qt5)
if(NOT QT_LRELEASE_EXECUTABLE)
    message(FATAL_ERROR "lrelease not found. Cannot compile translation files.")
endif()

# Create custom commands to compile each .ts file to .qm
set(QM_FILES)
foreach(TS_FILE ${TS_FILES})
    # Get the base name (e.g., ost_fr.ts -> ost_fr)
    get_filename_component(TS_BASENAME ${TS_FILE} NAME_WE)
    set(QM_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${TS_BASENAME}.qm)

    # Add custom command to generate .qm from .ts
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Compiling translation: ${TS_BASENAME}.ts -> ${TS_BASENAME}.qm"
        VERBATIM
    )

    list(APPEND QM_FILES ${QM_FILE})
endforeach()

# Create a custom target that depends on all .qm files
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# OST Server
add_executable(ostserver
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/globaldatastore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/globaldatastore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/globaldatastore.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translatemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translatemanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translations/translations.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfglobal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahi-qt/qt-watch_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahi-qt/qt-watch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahiclient.cpp
)
target_link_libraries(ostserver
    ostindimodule
#    ostmaincontrol
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
    ${avahi-client-lib}
    ${avahi-common-lib}
    Threads::Threads
    z
)
# Ensure translations are compiled before linking ostserver
add_dependencies(ostserver translations)
add_dependencies(ostserver generate_version)

#########################
#### modules libraries ##
#########################
# Base module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostbasemodule SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/datastore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/datastore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dbmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dbmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementtemplate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementstring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementfloat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementlight.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementlight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementimg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementimg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementvideo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementvideo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementtime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdatetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdatetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovsingle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertybase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertybase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertymulti.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertymulti.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertytextdumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertytextdumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyjsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/valuejsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/valuejsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovjsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/modulejsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/modulejsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/solver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/solver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/fileio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/fileio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/stretch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/stretch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/bayer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/bayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dms.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dms.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/nan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hh
)
set(BASE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/datastore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dbmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/solver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/fileio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/stretch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/bayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dms.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/nan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementtemplate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementlight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementimg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementvideo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementtime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementdatetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovsingle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertybase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/property/propertymulti.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovvisitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertytextdumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/valuejsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/lovjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/modulejsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.h
)
set_target_properties(ostbasemodule PROPERTIES PUBLIC_HEADER "${BASE_PUBLIC_HEADERS}")

target_link_libraries(ostbasemodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY}
    Threads::Threads
    z
)
add_dependencies(ostbasemodule generate_version)

# Indi module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule/indimodule.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostindimodule SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/indimodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/indimodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/indimodule.qrc
)
set(INDI_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/indimodule.h
)
set_target_properties(ostindimodule PROPERTIES PUBLIC_HEADER "${INDI_PUBLIC_HEADERS}")

target_link_libraries(ostindimodule
    ostbasemodule
    indiclient
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY}
    Threads::Threads
    z
)
add_dependencies(ostindimodule generate_version)



# Dummy module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy/dummy.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostdummy SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy/dummy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy/dummy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy/dummy.qrc
)

target_link_libraries(ostdummy PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostdummy PRIVATE dummy_LIBRARY)

# Maincontrol module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

#add_library(ostmaincontrol SHARED
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.qrc
#)
#target_link_libraries(ostmaincontrol PRIVATE
#    ostindimodule
#    Qt${QT_VERSION_MAJOR}::Core
#    Qt${QT_VERSION_MAJOR}::Concurrent
#    Qt${QT_VERSION_MAJOR}::Widgets
#    Threads::Threads
#    z
#)
#target_compile_definitions(ostmaincontrol PRIVATE maincontrol_LIBRARY)

# Indipanel module
add_library(ostindipanel SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel/indipanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel/indipanel.cpp
)
target_link_libraries(ostindipanel PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostindipanel PRIVATE INDIPANEL_MODULE)

# Focus module
add_library(ostfocus SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.h

)
target_link_libraries(ostfocus PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Scxml
    Threads::Threads
    z
)
target_compile_definitions(ostfocus PRIVATE FOCUS_MODULE)

# Sequence module
add_library(ostsequencer SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/sequencer/sequencer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/sequencer/sequencer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/sequencer/sequencer.qrc
)
target_link_libraries(ostsequencer PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Scxml
    Threads::Threads
    z
)
target_compile_definitions(ostsequencer PRIVATE SEQUENCER_MODULE)

# Guider module
add_library(ostguider SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/guider/guider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/guider/guider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/guider/guider.qrc
)
target_link_libraries(ostguider PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Scxml
    Threads::Threads
    z
)
target_compile_definitions(ostguider PRIVATE GUIDER_MODULE)

# Inspector module
add_library(ostinspector SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/inspector/inspector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/inspector/inspector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/inspector/inspector.qrc
)
target_link_libraries(ostinspector PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostinspector PRIVATE INSPECTOR_MODULE)

# Navigator module
add_library(ostnavigator SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/navigator/navigator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/navigator/navigator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/navigator/navigator.qrc
)
target_link_libraries(ostnavigator PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostnavigator PRIVATE INSPECTOR_MODULE)

# Allsky module
add_library(ostallsky SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/allsky/allsky.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/allsky/allsky.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/allsky/allsky.qrc
)
target_link_libraries(ostallsky PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostallsky PRIVATE INSPECTOR_MODULE)

# Planner module
add_library(ostplanner SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/planner/planner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/planner/planner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/planner/planner.qrc
)
target_link_libraries(ostplanner PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostplanner PRIVATE PLANNER_MODULE)

# Polar module
add_library(ostpolar SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/polar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/polar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/polar.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/rotations.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/rotations.cpp

)
target_link_libraries(ostpolar PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostpolar PRIVATE POLAR_MODULE)

install(TARGETS
    ostserver
    ostbasemodule ostindimodule
#    ostmaincontrol
    ostindipanel
    ostfocus
    ostsequencer
    ostguider
    ostinspector
    ostnavigator
    ostallsky
    ostplanner
    ostpolar
    RUNTIME DESTINATION "/usr/bin"     COMPONENT Runtime
    LIBRARY DESTINATION "/usr/lib"     COMPONENT Runtime
    PUBLIC_HEADER DESTINATION "/usr/include/ost" COMPONENT Development
)

# Generate version.cc at build time (not configure time)
# This ensures version information is always up-to-date with each build

# Create a custom target that always runs

add_custom_target(
    generate_version ALL
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/generate-version.sh"
            "${CMAKE_CURRENT_BINARY_DIR}/version.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cc.in"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/version.cc"
    COMMENT "Generating version.cc from git information..."
    VERBATIM
)

# Add version.cc to sources
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cc" version.hh)

message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "Version file will be generated at build time: ${CMAKE_CURRENT_BINARY_DIR}/version.cc")

# ---------------------------------------------------------------------------
# Packaging (CPack / Debian)
# ---------------------------------------------------------------------------
include(cmake/cpack/DebianPackage.cmake)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


