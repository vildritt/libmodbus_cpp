cmake_minimum_required(VERSION 2.8)

project(modbus_cpp)

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
)

if(DEFINED USE_QT5)
    find_package(Qt5Core)
    find_package(Qt5Network)
    find_package(Qt5SerialPort)

    set(SOURCE_LIB_RTU
        libmodbus_cpp/slave_rtu.cpp
        libmodbus_cpp/slave_rtu_backend.cpp
        libmodbus_cpp/master_rtu_backend.cpp
        libmodbus_cpp/master_rtu.cpp
    )
else()
    find_package(Qt4 COMPONENTS QTCORE QTNETWORK REQUIRED)
    include (${QT_USE_FILE})
    set(SOURCE_LIB_RTU)
    add_definitions( "-DQ_NULLPTR=nullptr" )
endif()

add_library(modbus_cpp SHARED ${SOURCE_LIB} ${SOURCE_LIB_RTU})

if(DEFINED USE_QT5)
    qt5_use_modules(modbus_cpp Core)
    qt5_use_modules(modbus_cpp Network)
    qt5_use_modules(modbus_cpp SerialPort)
endif()




