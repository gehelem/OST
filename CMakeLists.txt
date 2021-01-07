cmake_minimum_required(VERSION 3.0)
PROJECT(OST C CXX)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})
include_directories(${CFITSIO_INCLUDE_DIR})
include_directories(${GSL_INCLUDE_DIRS})
include_directories(${CIMG_INCLUDE_DIRS})
include_directories(${Qt5_DIR})
include_directories(${QT5Declarative_INCLUDE_LIBRARIES} )
include_directories(${QT5Declarative_INCLUDE_DIRS} )
include_directories(${STELLARSOLVER_INCLUDE_DIR})
include(CMakeCommon)

SET( APP_EXE OSTServer )

add_executable( ${APP_EXE}
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTController.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTImage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientGEN.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientCTL.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientPAN.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientFOC.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientGUI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientSEQ.cpp
    )

SET( APP_EXETEST tester )

add_executable( ${APP_EXETEST}
        ${CMAKE_CURRENT_SOURCE_DIR}/job.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apptester.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OSTImage.cpp
    )

target_link_libraries( ${APP_EXE}
    indiclientqt
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    Qt5::WebSockets
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -lpthread
    )

target_link_libraries( ${APP_EXETEST}
    indiclientqt
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    Qt5::WebSockets
    ${STELLARSOLVER_LIBRARIES}
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
