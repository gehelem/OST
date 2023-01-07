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
find_package(Boost COMPONENTS log log_setup REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule)
include_directories(${INDI_INCLUDE_DIR})
include_directories(${CFITSIO_INCLUDE_DIR})
include_directories(${GSL_INCLUDE_DIRS})
include_directories(${Qt5_DIR})
include_directories(${QT5Declarative_INCLUDE_LIBRARIES} )
include_directories(${QT5Declarative_INCLUDE_DIRS} )
include_directories(${STELLARSOLVER_INCLUDE_DIR})

include(CMakeCommon)


# OST Server
add_executable(ostserver
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dbmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dbmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc
)
target_link_libraries(ostserver
    ostbasemodule
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Sql
    Threads::Threads
    z
)

#########################
#### modules libraries ##
#########################
# Base module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostbasemodule STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/basemodule/basemodule.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/solver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/solver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/fileio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/imagelabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/stretch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/bayer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssolverutils/dms.cpp
)
target_link_libraries(ostbasemodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY}
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)

# Indi module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule/indimodule.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostindimodule SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule/indimodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule/indimodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indimodule/indimodule.qrc
)
target_link_libraries(ostindimodule
    ostbasemodule
    indiclient
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY}
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)



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
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostdummy PRIVATE dummy_LIBRARY)

# Maincontrol module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})

add_library(ostmaincontrol SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/maincontrol/maincontrol.qrc
)
target_link_libraries(ostmaincontrol PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostmaincontrol PRIVATE maincontrol_LIBRARY)


# Focus module
#list (APPEND RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.qrc)
#qt5_add_resources (RCC_SOURCES ${RESOURCES})
add_library(ostfocuser SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.h
    ${RCC_SOURCES}
)
target_link_libraries(ostfocuser PRIVATE
    ostindimodule
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostfocuser PRIVATE FOCUS_MODULE)


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
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostindipanel PRIVATE INDIPANEL_MODULE)


# guider module
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
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostguider PRIVATE GUIDER_MODULE)

## polar module
#add_library(ostpolar SHARED
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/polar.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/polar.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/rotations.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/polar/rotations.cpp
#)
#target_link_libraries(ostpolar PRIVATE
#    indiclient
#    Qt${QT_VERSION_MAJOR}::Core
#    Qt${QT_VERSION_MAJOR}::Concurrent
#    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
#    ${STELLARSOLVER_LIBRARIES}
#    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
#    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
#    z -lpthread
#)
#target_compile_definitions(ostpolar PRIVATE POLAR_MODULE)
#
# inspector module
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
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostinspector PRIVATE INSPECTOR_MODULE)

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
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Threads::Threads
    z
)
target_compile_definitions(ostallsky PRIVATE ALLSKY_MODULE)



install(TARGETS ostserver ostbasemodule ostindimodule ostdummy ostallsky ostinspector ostguider ostindipanel ostfocuser ostmaincontrol
    RUNTIME DESTINATION "/usr/bin"
    LIBRARY DESTINATION "/usr/bin"
)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
