cmake_minimum_required(VERSION 3.0)
PROJECT(OST C CXX)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#find_package(Qt5Core)
find_package(CFITSIO REQUIRED)
find_package(INDI REQUIRED)
#find_package(INDI COMPONENTS clientqt5 REQUIRED)
find_package(ZLIB REQUIRED)
#find_package(Threads REQUIRED)
find_package(Nova REQUIRED)
find_package(GSL REQUIRED)
find_package(OpenCV REQUIRED )
find_package(CImg REQUIRED )
find_package(WCSLIB REQUIRED)
find_package(Qt5 5.15.2 REQUIRED COMPONENTS Gui Widgets Core Concurrent Network WebSockets Scxml Sql)
find_package(StellarSolver REQUIRED)
find_package(Boost COMPONENTS log log_setup REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/modules)
include_directories(${INDI_INCLUDE_DIR})
include_directories(${CFITSIO_INCLUDE_DIR})
include_directories(${GSL_INCLUDE_DIRS})
include_directories(${CIMG_INCLUDE_DIRS})
include_directories(${Qt5_DIR})
include_directories(${QT5Declarative_INCLUDE_LIBRARIES} )
include_directories(${QT5Declarative_INCLUDE_DIRS} )
include_directories(${STELLARSOLVER_INCLUDE_DIR})

include(CMakeCommon)


# OST Server
add_executable( ost
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/properties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/basemodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/basemodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image.h

)
target_link_libraries( ost
    indiclient
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Scxml
    Qt${QT_VERSION_MAJOR}::Sql
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -lpthread
    )

# Focus module
add_library(focuser SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/focus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus/polynomialfit.h
)
target_link_libraries(focuser PRIVATE
    indiclient
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
)
target_compile_definitions(focuser PRIVATE FOCUSER_LIBRARY)

# Indipanel module
add_library(indipanel SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel/indipanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/indipanel/indipanel.cpp
)
target_link_libraries(indipanel PRIVATE
    indiclient
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
)
target_compile_definitions(indipanel PRIVATE INDIPANEL_LIBRARY)


#install(TARGETS HeadlessObservatory RUNTIME DESTINATION bin)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
