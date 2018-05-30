cmake_minimum_required(VERSION 3.0)

project(rpiDisplay)

# To use this build for cross-compilation with the created kit (Generic Linux),
# source the environment before starting QtCreator
#
# i.e:
# #!/bin/bash
# source /opt/embeddev/0.1.0/environment-setup-arm1176jzfshf-vfp-embeddev-linux-gnueabi && sleep 2 && ~/Qt5_10/Tools/QtCreator/bin/qtcreator
#

# The following is given as parameter (-D) by QtCreator
#set (CMAKE_CXX_COMPILER "/opt/embeddev/0.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-embeddev-linux-gnueabi/arm-embeddev-linux-gnueabi-g++")
#set (CMAKE_C_COMPILER "/opt/embeddev/0.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-embeddev-linux-gnueabi/arm-embeddev-linux-gnueabi-gcc")
#set (CMAKE_PREFIX_PATH "/opt/embeddev/0.1.0/sysroots/arm1176jzfshf-vfp-embeddev-linux-gnueabi/usr")
#set (QT_QMAKE_EXECUTABLE "/opt/embeddev/0.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/qt5/qmake")


### CROSS COMPILING for Raspberry Pi
# Set if you want to cross compile these..
#set(toolchain_location "/home/mozcelikors/RaspberryPiToolchain")
#set(CMAKE_TOOLCHAIN_FILE "${toolchain_location}/crossToolchainRpi.cmake")
# If not, you can deploy and remotely build in raspberry pi
###

# Following is important to make sure OpenEmbedded Qt Toolchain is set
set (OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS} )
set (QMAKE_CFLAGS_ISYSTEM "-I")

# Verbose
set( CMAKE_VERBOSE_MAKEFILE on )

set (CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS "-std=c++11 -fPIC -Wno-write-strings -Wno-deprecated-declarations -Wno-unused-local-typedefs -O2 -pipe -g -feliminate-unused-debug-types  -fstack-protector-strong -pie -fpie -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -fvisibility-inlines-hidden ${CMAKE_CXX_FLAGS}")

if(DEFINED ENV{SDKTARGETSYSROOT})
    message(STATUS "SDK Detected")
    set(SDK_EN 1)
    # Use OpenCV config from Yocto SDK (WA)
    # set(OpenCV_DIR $ENV{SDKTARGETSYSROOT}/usr/share/OpenCV)
    # Raspicam module
    set(CMAKE_MODULE_PATH $ENV{SDKTARGETSYSROOT}/usr/lib/cmake ${CMAKE_MODULE_PATH})

else(DEFINED ENV{SDKTARGETSYSROOT})
    set(SDK_EN 0)
endif(DEFINED ENV{SDKTARGETSYSROOT})

# Find the QtWidgets library
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(raspicam REQUIRED)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)

# Add a compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

## COMMON =====================================================
set (PROJECT_BUILD_DIR "${PROJECT_SOURCE_DIR}/build")
include_directories ("${PROJECT_SOURCE_DIR}/common/include")

install(FILES "${PROJECT_SOURCE_DIR}/scripts/start.sh"
  DESTINATION "/opt/rpiDisplay/")

## FRONTEND ===================================================
# Generate code from ui files
qt5_wrap_cpp(frontend_app_SRC "${PROJECT_SOURCE_DIR}/frontend/include/mainwindow.h")
qt5_wrap_cpp(CustomLineEdit_SRC "${PROJECT_SOURCE_DIR}/frontend/include/CustomLineEdit.h")
qt5_wrap_ui(frontend_app_UI "${PROJECT_SOURCE_DIR}/frontend/gui/mainwindow.ui")
# Generate rules for building source files from the resources
qt5_add_resources(frontend_app_QRCS "${PROJECT_SOURCE_DIR}/frontend/resources/resources.qrc")
add_executable(frontend_app "${PROJECT_SOURCE_DIR}/frontend/src/main.cpp"
                        "${PROJECT_SOURCE_DIR}/frontend/src/mainwindow.cpp"
                        ${frontend_app_UI}
                        ${frontend_app_SRC}
                        ${frontend_app_QRCS}
                        "${PROJECT_SOURCE_DIR}/frontend/src/CustomLineEdit.cpp"
                        ${CustomLineEdit_SRC})

qt5_wrap_cpp(backend_recv_SRC "${PROJECT_SOURCE_DIR}/frontend/include/BackendReceiveThread.h")
add_library (backend_recv SHARED "${PROJECT_SOURCE_DIR}/frontend/src/BackendReceiveThread.cpp" ${backend_recv_SRC})
target_include_directories (backend_recv PUBLIC "${PROJECT_SOURCE_DIR}/frontend/include")
set_property(TARGET backend_recv PROPERTY NO_SYSTEM_FROM_IMPORTED ON)
target_link_libraries(backend_recv Qt5::Widgets)
target_link_libraries(backend_recv rt)

qt5_wrap_cpp(camera_SRC "${PROJECT_SOURCE_DIR}/frontend/include/CameraThread.h")
add_library (camera SHARED "${PROJECT_SOURCE_DIR}/frontend/src/CameraThread.cpp" ${camera_SRC})
target_include_directories (camera PUBLIC "${PROJECT_SOURCE_DIR}/frontend/include")
set_property(TARGET camera PROPERTY NO_SYSTEM_FROM_IMPORTED ON)
target_link_libraries(camera Qt5::Widgets)
target_link_libraries(camera raspicam)
target_link_libraries(camera raspicam_cv)
target_link_libraries(camera opencv_core)
target_link_libraries(camera opencv_highgui)
target_link_libraries(camera opencv_imgproc)
target_link_libraries(camera opencv_imgcodecs)
# Needed for camera component to work:
target_link_libraries(camera mmal)
target_link_libraries(camera mmal_core)
target_link_libraries(camera mmal_util)
target_link_libraries(camera pthread)

target_include_directories (frontend_app PUBLIC "${PROJECT_SOURCE_DIR}/frontend/include")
target_link_libraries(frontend_app Qt5::Widgets)
target_link_libraries(frontend_app backend_recv)
target_link_libraries(frontend_app wiringPi)
target_link_libraries(frontend_app pthread)
target_link_libraries(frontend_app camera)
# The following line is the life-saver to remove system includes (-isystem) that are causing lots of problems..
set_property(TARGET frontend_app PROPERTY NO_SYSTEM_FROM_IMPORTED ON)


## BACKEND ===================================================
add_library (rtlib SHARED "${PROJECT_SOURCE_DIR}/backend/src/rtlib/rtlib.c")
target_include_directories (rtlib PUBLIC "${PROJECT_SOURCE_DIR}/backend/include/rtlib")
target_include_directories (rtlib PUBLIC "${PROJECT_SOURCE_DIR}/backend/include")
target_link_libraries (rtlib pthread)
target_link_libraries (rtlib rt)

# Tell CMake to create the executable
add_executable(backend_app "${PROJECT_SOURCE_DIR}/backend/src/main.cpp")
target_include_directories (backend_app PUBLIC "${PROJECT_SOURCE_DIR}/backend/include")
target_link_libraries (backend_app rtlib)
target_link_libraries (backend_app pthread)
target_link_libraries (backend_app rt)
target_link_libraries (backend_app wiringPi)
