LIBMODBUS_CPP_TARGET = modbus_cpp
LIBMODBUS_CPP_DESTDIR = bin
LIBMODBUS_CPP_CONFIG = \
    libmodbus_cpp_tests \
    dll
LIBMODBUS_CPP_HEADERS =  $${PWD}


LIBMODBUS_TARGET = modbus
LIBMODBUS_HEADERS =  $${PWD}/libmodbus/include


include(lxqmake.pri): {
    LIB_USER_CONF_FILES = $$lxqmt_getUserConfs($${PWD})
    for(LIB_USER_CONF_FILE, LIB_USER_CONF_FILES):include($$LIB_USER_CONF_FILE)
}



QT += \
    network \
    serialport

INCLUDEPATH += \
    $${LIBMODBUS_CPP_HEADERS} \
    $${LIBMODBUS_HEADERS}

LIBMODBUS_CPP_LIB = -L$${LIBMODBUS_CPP_DESTDIR} -l$${LIBMODBUS_CPP_TARGET}
LIBMODBUS_LIB     = -L$${LIBMODBUS_CPP_DESTDIR} -l$${LIBMODBUS_TARGET}
