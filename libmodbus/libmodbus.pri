LIBMODBUS_TARGET  = modbus
LIBMODBUS_DESTDIR = bin
LIBMODBUS_HEADERS = $${PWD}/include
LIBMODBUS_CONFIG  = dll


include($${PWD}/../lxqmake.pri,): {
    LIB_USER_CONF_FILES = $$lxqmt_getUserConfs($${PWD})
    for(LIB_USER_CONF_FILE, LIB_USER_CONF_FILES):include($$LIB_USER_CONF_FILE)
}


LIBMODBUS_LIB      = -L$${LIBMODBUS_DESTDIR} -l$${LIBMODBUS_TARGET}

INCLUDEPATH       += $${LIBMODBUS_HEADERS}
