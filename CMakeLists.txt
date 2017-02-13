cmake_minimum_required(VERSION 2.8)

project(libmodbus_cpp)

include_directories(libmodbus/include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_LIB
    libmodbus_cpp/backend.cpp
    libmodbus_cpp/slave_tcp.cpp
    libmodbus_cpp/abstract_slave.cpp
    libmodbus_cpp/abstract_master.cpp
    libmodbus_cpp/slave_tcp_backend.cpp
    libmodbus_cpp/master_tcp_backend.cpp
    libmodbus_cpp/master_tcp.cpp
    libmodbus_cpp/factory.cpp
    libmodbus_cpp/slave_rtu.cpp
    #libmodbus_cpp/slave_rtu_backend.cpp
    #libmodbus_cpp/master_rtu_backend.cpp
    #libmodbus_cpp/master_rtu.cpp
    )

find_package(Qt5Core)


#find_package(Qt4 COMPONENTS QTCORE QTNETWORK REQUIRED)
#include (${QT_USE_FILE})

add_library(libmodbus_cpp SHARED ${SOURCE_LIB})
#target_link_libraries(libmodbus_cpp  ${QT_LIBRARIES})
