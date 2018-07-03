LIBMODBUS_CPP_TARGET = modbus_cpp
LIBMODBUS_CPP_DESTDIR = bin
LIBMODBUS_CPP_CONFIG = libmodbus_cpp_tests
LIBMODBUS_CPP_CONFIG += dll
LIBMODBUS_HEADERS = $${PWD}/libmodbus/include

INCLUDEPATH += \
    $${PWD} \
    $${PWD}/libmodbus/include

include(lxqmake.pri): {
    LIB_USER_CONF_FILES = $$lxqmt_getUserConfs($${PWD})
    for(LIB_USER_CONF_FILE, LIB_USER_CONF_FILES):include($$LIB_USER_CONF_FILE)
}

QT += network serialport
LIBMODBUS_LIB = -L$${LIBMODBUS_CPP_DESTDIR} -lmodbus
