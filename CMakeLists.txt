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

# Following is important to make sure OpenEmbedded Qt Toolchain is set
set (OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS} )
set (QMAKE_CFLAGS_ISYSTEM "-I")
# Verbose
set( CMAKE_VERBOSE_MAKEFILE on )


# Find the QtWidgets library
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

### CROSS COMPILING for Raspberry Pi
# Set if you want to cross compile these..
#set(toolchain_location "/home/mozcelikors/RaspberryPiToolchain")
#set(CMAKE_TOOLCHAIN_FILE "${toolchain_location}/crossToolchainRpi.cmake")
# If not, you can deploy and remotely build in raspberry pi
###

set (CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS "-std=c++11 -fPIC -Wno-write-strings -Wno-deprecated-declarations -Wno-unused-local-typedefs -O2 -pipe -g -feliminate-unused-debug-types  -fstack-protector-strong -pie -fpie -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -fvisibility-inlines-hidden ${CMAKE_CXX_FLAGS}")

if(DEFINED ENV{SDKTARGETSYSROOT})
    message(STATUS "SDK Detected")
    set(SDK_EN 1)

    # Use OpenCV config from Yocto SDK (WA)
    set(OpenCV_DIR $ENV{SDKTARGETSYSROOT}/usr/share/OpenCV)
    # Raspicam module
    set(CMAKE_MODULE_PATH $ENV{SDKTARGETSYSROOT}/usr/lib/cmake ${CMAKE_MODULE_PATH})

else(DEFINED ENV{SDKTARGETSYSROOT})
    set(SDK_EN 0)
endif(DEFINED ENV{SDKTARGETSYSROOT})

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)

# There may be a way to tell up front if Qt5 is going to be found, but I haven't found
# a foolproof way to do it yet, so settle for the default error message for now.
#if(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)
#    message(WARNING "CMAKE_PREFIX_PATH is not defined, so find_package may not work. Set the CMAKE_PREFIX_PATH "
#            "environment variable to the install prefix of Qt 5, either on the command line as "
#            "-DCMAKE_PREFIX_PATH=\"path/to/Qt5/lib/cmake\" or with set(CMAKE_PREFIX_PATH path/to/Qt5/lib/cmake)")
#endif(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)

# Add a compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

## NON-QT SECTION-----------------
# Path to external libraries for linking
#set (wiringPi_includepath "${toolchain_location}/extlibs/wiringPi/wiringPi")
#set (wiringPi_buildpath "${toolchain_location}/extlibs/wiringPi/wiringPi/build")
# Point to CMake path script where to find Qt libraries in the target (i.e find scripts)
#set (Qt5Core_DIR "${toolchain_location}/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib/arm-linux-gnueabihf/cmake/Qt5Core/")
#set (Qt5Widgets_DIR "${toolchain_location}/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib/arm-linux-gnueabihf/cmake/Qt5Widgets/")
# Include external libraries
#include_directories (include ${wiringPi_includepath})
# Actually find the wiringPi library object
#find_library(WPI_LIB wiringPi HINTS ${wiringPi_buildpath} NO_CMAKE_FIND_ROOT_PATH)
# Alert the user if we do not find it
#if(NOT WPI_LIB)
#  message(FATAL_ERROR “wiringPi library not found”)
#endif()

## -------------------NON-QT SECTION

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# Generate code from ui files
qt5_wrap_ui(UI_HEADERS mainwindow.ui)

# Generate rules for building source files from the resources
qt5_add_resources(QRCS resources/resources.qrc)

# Tell CMake to create the helloworld executable
add_executable(rpiDisplay main.cpp mainwindow.cpp ${UI_HEADERS} ${QRCS})

# Add the Qt5 Widgets for linking
target_link_libraries(rpiDisplay Qt5::Widgets)

# The following line is the life-saver to remove system includes (-isystem) that are causing lots of problems..
set_property(TARGET rpiDisplay PROPERTY NO_SYSTEM_FROM_IMPORTED ON)

## NON-QT SECTION-----------------

#Link external libraries
#target_link_libraries(rpiDisplay ${WPI_LIB})
target_link_libraries (rpiDisplay wiringPi)
## -------------------NON-QT SECTION
