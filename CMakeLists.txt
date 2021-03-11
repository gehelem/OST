cmake_minimum_required(VERSION 3.0)
PROJECT(OST C CXX)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

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
find_package(Qt5 5.4 REQUIRED COMPONENTS Gui Widgets Core Concurrent Network WebSockets)
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


SET( APP_EXEOST ost )

add_executable( ${APP_EXEOST}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/focus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/indiclient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ost.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/wshandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/properties.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/polynomialfit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/value.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/textvalue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/switchvalue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/numbervalue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/setup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/setup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/device.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/property.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/propertyvisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/numberproperty.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/propertyfactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/propertyfactory.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/newpropertylogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/newpropertylogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/propertytextdumper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/propertytextdumper.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/updatedpropertylogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/updatedpropertylogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/switchproperty.h)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

target_link_libraries( ${APP_EXEOST}
    indiclient
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    Qt5::WebSockets
    ${STELLARSOLVER_LIBRARIES} ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -lpthread
    )

#install(TARGETS HeadlessObservatory RUNTIME DESTINATION bin)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
