TARGET = modbus

QMAKE_CFLAGS += -Wno-all -Wno-unused -Wno-format
INCLUDEPATH += $${PWD}

include(../lxqmake.pri) {
    LIB_USER_CONF_FILES = $$lxqmt_getUserConfs($${PWD})
    for(LIB_USER_CONF_FILE, LIB_USER_CONF_FILES):include($$LIB_USER_CONF_FILE)
}

unix {
    # http://stackoverflow.com/questions/3612283/running-a-program-script-from-qmake
    TEMPLATE=aux
    OTHER_FILES += build_libmodbus.sh
    build_libmodbus.commands = $${PWD}/build_libmodbus.sh $${PWD}/libmodbus $$LIBMODBUS_CPP_DESTDIR
    QMAKE_EXTRA_TARGETS += build_libmodbus
    PRE_TARGETDEPS += build_libmodbus
}
win32 {
    TEMPLATE = lib
    CONFIG += dll
    CONFIG -= qt

    SOURCES += \
        libmodbus/src/modbus.c \
        libmodbus/src/modbus-data.c \
        libmodbus/src/modbus-rtu.c \
        libmodbus/src/modbus-tcp.c

    HEADERS += \
        libmodbus/src/modbus-version.h \
        libmodbus/src/modbus.h \
        libmodbus/src/modbus-private.h \
        libmodbus/src/modbus-rtu.h \
        libmodbus/src/modbus-rtu-private.h \
        libmodbus/src/modbus-tcp.h \
        libmodbus/src/modbus-tcp-private.h \
        libmodbus/src/config.h

    INCLUDEPATH += $${PWD}/libmodbus/src

    LIBS += \
        -lwsock32 \
        -lws2_32

    DESTDIR = $${LIBMODBUS_CPP_DESTDIR}
}
