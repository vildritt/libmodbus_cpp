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
    libmodbus_cpp/slave_rtu_backend.cpp
    libmodbus_cpp/master_rtu_backend.cpp
    libmodbus_cpp/master_rtu.cpp
    )

find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5SerialPort)

add_library(libmodbus_cpp SHARED ${SOURCE_LIB})

qt5_use_modules(libmodbus_cpp Core)
qt5_use_modules(libmodbus_cpp Network)
qt5_use_modules(libmodbus_cpp SerialPort)



