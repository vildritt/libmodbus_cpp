TEMPLATE = subdirs

include(libmodbus_cpp.pri)

SUBDIRS += \
    libmodbus \
    libmodbus_cpp

libmodbus_cpp.depends = libmodbus

DEFINES += USE_QT5

contains(LIBMODBUS_CPP_CONFIG, libmodbus_cpp_tests) {
    SUBDIRS += tests
    tests.depends = libmodbus_cpp
}

OTHER_FILES += \
    libmodbus_cpp.prf \
    user_conf.pri.template
