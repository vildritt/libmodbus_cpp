cmake_minimum_required(VERSION 2.8.6)

project(modbus_cpp)

set(LIBMODBUS_INCLUDE_DIRS       ${CMAKE_CURRENT_SOURCE_DIR}/libmodbus/include PARENT_SCOPE)
set(LIBMODBUS_CPP_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}                   PARENT_SCOPE)
set(LIBMODBUS_CPP_LIBRARIES_DIR  ${CMAKE_CURRENT_BINARY_DIR}                   PARENT_SCOPE)
set(LIBMODBUS_CPP_LIBRARIES     -lmodbus_cpp -lmodbus                          PARENT_SCOPE)

set(CMAKE_AUTOMOC ON)

include_directories(libmodbus/include)
include_directories(libmodbus/include/modbus) # to make available "modbus-version.h"e
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
    cmake_policy(SET CMP0020 NEW)
endif()

set(SOURCE_LIB
    libmodbus_cpp/backend.cpp
    libmodbus_cpp/slave_tcp.cpp
    libmodbus_cpp/abstract_slave.cpp
    libmodbus_cpp/abstract_master.cpp
    libmodbus_cpp/slave_tcp_backend.cpp
    libmodbus_cpp/master_tcp_backend.cpp
    libmodbus_cpp/master_tcp.cpp
    libmodbus_cpp/factory.cpp
    libmodbus_cpp/global.cpp
    libmodbus_cpp/mapping_wrapper.h
    libmodbus_cpp/logger.h
)

set(TESTS_APP
    tests/main.cpp
    tests/reg_map_read_write_test.cpp
    tests/abstract_read_write_test.cpp
    tests/tcp_read_write_test.cpp
#    tests/rtu_read_write_test.cpp
)

if(DEFINED USE_QT5)
    find_package(Qt5Core)
    find_package(Qt5Network)
    find_package(Qt5SerialPort)
    find_package(Qt5Test)

    set(SOURCE_LIB_RTU
        libmodbus_cpp/slave_rtu.cpp
        libmodbus_cpp/slave_rtu_backend.cpp
        libmodbus_cpp/master_rtu_backend.cpp
        libmodbus_cpp/master_rtu.cpp
    )
else()
    find_package(Qt4 COMPONENTS QTCORE QTNETWORK QTTEST REQUIRED)
    include (${QT_USE_FILE})
    set(SOURCE_LIB_RTU)
    add_definitions( "-DQ_NULLPTR=nullptr" )
endif()

add_library(modbus_cpp SHARED ${SOURCE_LIB} ${SOURCE_LIB_RTU})

set(MODBUS_LIB
    libmodbus/libmodbus/src/modbus.c
    libmodbus/libmodbus/src/modbus-data.c
    libmodbus/libmodbus/src/modbus-rtu.c
    libmodbus/libmodbus/src/modbus-tcp.c
    )

add_library(modbus SHARED ${MODBUS_LIB})

if(NOT USE_IWYU)
add_dependencies(modbus_cpp modbus)
endif()

link_directories(${LIBMODBUS_CPP_LIBRARIES_DIR})

if(LIBMODBUSCPP_TESTS)
    if(NOT WIN32)
        add_executable(modbus_tests ${TESTS_APP} ${SOURCE_LIB})
        target_link_libraries(modbus_tests ${LIBMODBUS_CPP_LIBRARIES} ${QT_LIBRARIES})
        if(NOT USE_IWYU)
            add_dependencies(modbus_tests modbus_cpp)
        endif()
    endif()
endif()

if(NOT DEFINED USE_QT5)
target_link_libraries(modbus_cpp ${QT_LIBRARIES})
endif()

if(DEFINED USE_QT5)

    if(DEFINED USE_QT5)
        target_link_libraries(modbus_cpp Qt5::Core Qt5::Network Qt5::SerialPort Qt5::Test)
    else()
        qt5_use_modules(modbus_cpp Core)
        qt5_use_modules(modbus_cpp Network)
        qt5_use_modules(modbus_cpp SerialPort)
        qt5_use_modules(modbus_cpp Test)
    endif()

    if(LIBMODBUSCPP_TESTS)
        if(NOT WIN32)
            if(DEFINED USE_QT5)
                target_link_libraries(modbus_tests Qt5::Core Qt5::Network Qt5::SerialPort Qt5::Test)
            else()
                qt5_use_modules(modbus_tests Core)
                qt5_use_modules(modbus_tests Network)
                qt5_use_modules(modbus_tests SerialPort)
                qt5_use_modules(modbus_tests Test)
            endif()

        endif()
    endif()
    add_definitions(-DUSE_QT5)
endif()

if(WIN32)
    target_link_libraries(
        modbus -lWs2_32
    )
    link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

target_link_libraries(modbus_cpp modbus)

set_source_files_properties(libmodbus/libmodbus/src/modbus-tcp.c PROPERTIES COMPILE_FLAGS -w)


install (
    TARGETS modbus_cpp modbus
    DESTINATION lib)

