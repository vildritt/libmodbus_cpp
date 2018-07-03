CONFIG -= qt

include(libmodbus.pri)

TARGET = $${LIBMODBUS_TARGET}
CONFIG += $${LIBMODBUS_CONFIG}

QMAKE_CFLAGS += -Wno-all -Wno-unused -Wno-format

LIBMODBUS_SRC = libmodbus/src


unix {
    # http://stackoverflow.com/questions/3612283/running-a-program-script-from-qmake

    TEMPLATE = aux

    build_libmodbus.commands = "$${PWD}/build_libmodbus.sh" "$${PWD}/libmodbus" "$${LIBMODBUS_DESTDIR}"

    QMAKE_EXTRA_TARGETS += build_libmodbus
    PRE_TARGETDEPS += build_libmodbus

    OTHER_FILES += build_libmodbus.sh
}


win32 {
    TEMPLATE = lib

    SOURCES += \
        $${LIBMODBUS_SRC}/modbus.c \
        $${LIBMODBUS_SRC}/modbus-data.c \
        $${LIBMODBUS_SRC}/modbus-rtu.c \
        $${LIBMODBUS_SRC}/modbus-tcp.c

    HEADERS += \
        $${LIBMODBUS_SRC}/modbus-version.h \
        $${LIBMODBUS_SRC}/modbus.h \
        $${LIBMODBUS_SRC}/modbus-private.h \
        $${LIBMODBUS_SRC}/modbus-rtu.h \
        $${LIBMODBUS_SRC}/modbus-rtu-private.h \
        $${LIBMODBUS_SRC}/modbus-tcp.h \
        $${LIBMODBUS_SRC}/modbus-tcp-private.h \
        $${LIBMODBUS_SRC}/config.h

    INCLUDEPATH += 
        $${LIBMODBUS_SRC}

    LIBS += \
        -lwsock32 \
        -lws2_32

    DESTDIR = $${LIBMODBUS_DESTDIR}
}
