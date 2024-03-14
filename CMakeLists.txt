cmake_minimum_required(VERSION 3.0)
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
find_package(QT NAMES Qt5 COMPONENTS Gui Widgets Core Concurrent Network WebSockets Sql REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Gui Widgets Core Concurrent Network WebSockets Sql)
find_package(StellarSolver REQUIRED)
find_library(avahi-client-lib avahi-client REQUIRED)
find_library(avahi-common-lib avahi-common REQUIRED)
find_path(avahi-client-includes avahi-client/client.h REQUIRED)
find_path(avahi-common-includes avahi-common/defs.h REQUIRED)




include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules)
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


# OST Server
add_executable(ostserver
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/datastore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/datastore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dbmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dbmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfglobal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/qzeroconfservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahi-qt/qt-watch_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahi-qt/qt-watch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QtZeroConf/avahiclient.cpp
)
target_link_libraries(ostserver
    ostbasemodule
    ostmaincontrol
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    ${avahi-client-lib}
    ${avahi-common-lib}
    Threads::Threads
    z
)

#########################
#### modules libraries ##
#########################
# Base module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostbasemodule SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/baseroot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/baseroot.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementsingle.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementmessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementmessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementgraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementgraph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovsingle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovlight.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementtextdumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementtextdumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementset.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/baseroot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/solver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/fileio.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/stretch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/bayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dms.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/nan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementsingle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementbool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementlight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementimg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementvideo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementmessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementgraph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/element/elementprg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovbase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovsingle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovfloat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/model/listofvalues/lovlight.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementtextdumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementjsondumper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/propertyupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementupdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utils/elementset.h
)
set_target_properties(ostbasemodule PROPERTIES PUBLIC_HEADER "${BASE_PUBLIC_HEADERS}")

target_link_libraries(ostbasemodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY}
    Threads::Threads
    z
)

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



# Dummy module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy/dummy.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostdummy SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/dummy.qrc
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

add_library(ostmaincontrol SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol.qrc
)
target_link_libraries(ostmaincontrol PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
    z
)
target_compile_definitions(ostmaincontrol PRIVATE maincontrol_LIBRARY)

# Indipanel module
add_library(ostindipanel SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel.cpp
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


install(TARGETS
    ostserver
    ostbasemodule ostindimodule ostdummy ostmaincontrol ostindipanel
    RUNTIME DESTINATION "/usr/bin"
    LIBRARY DESTINATION "/usr/lib"
    PUBLIC_HEADER DESTINATION "/usr/include/ost"
)

# the commit's SHA1, and whether the building workspace was dirty or not
execute_process(COMMAND
  "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# generate version.cc
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cc.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cc" @ONLY)
#list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cc" version.hh)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cc" version.hh)


message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "GIT_SHA1=${GIT_SHA1}")
message(STATUS "GIT_DATE=${GIT_DATE}")
message(STATUS "GIT_COMMIT_SUBJECT=${GIT_COMMIT_SUBJECT}")


#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


