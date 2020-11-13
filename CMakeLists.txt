cmake_minimum_required(VERSION 3.0)
PROJECT(OST C CXX)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(CFITSIO REQUIRED)
find_package(INDI COMPONENTS client REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(Nova REQUIRED)
find_package(GSL REQUIRED)
find_package(OpenCV REQUIRED )
find_package(CImg REQUIRED )
find_package(WCSLIB REQUIRED)
find_package(Qt5 5.4 REQUIRED COMPONENTS Gui Widgets Core Concurrent Network)
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


SET( APP_LIB_IMG OSTImage )
add_library(${APP_LIB_IMG} ${CMAKE_CURRENT_SOURCE_DIR}/OSTImage.cpp
    )
target_link_libraries( ${APP_LIB_IMG}  ${STELLARSOLVER_LIBRARIES} indiclient
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -pthread
    )

SET( APP_LIB_GEN OSTClientGEN )
add_library(${APP_LIB_GEN} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientGEN.cpp
    )
target_link_libraries( ${APP_LIB_GEN}  indiclient
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -pthread
    )

SET( APP_LIB_FOC OSTClientFOC )
add_library(${APP_LIB_FOC} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientFOC.cpp)
target_link_libraries( ${APP_LIB_FOC}  indiclient
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -pthread
    )

SET( APP_LIB_SEQ OSTClientSEQ )
add_library(${APP_LIB_SEQ} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/OSTClientSEQ.cpp)
target_link_libraries( ${APP_LIB_SEQ}  indiclient
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${STELLARSOLVER_LIBRARIES}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -pthread
    )



SET( APP_EXE OSTController )

add_executable( ${APP_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/OSTController.cpp
    )

target_link_libraries( ${APP_EXE}  indiclient
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${STELLARSOLVER_LIBRARIES}
    ${APP_LIB_GEN} ${APP_LIB_SEQ} ${APP_LIB_FOC} ${APP_LIB_IMG}
    ${INDI_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARY}  ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES} ${ZLIB_LIBRARY} ${CIMG_LIBRARIES} ${OpenCV_LIBS}
    z -pthread
    )
#install(TARGETS HeadlessObservatory RUNTIME DESTINATION bin)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
