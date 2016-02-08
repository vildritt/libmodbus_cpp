#ifndef DEFS_H
#define DEFS_H

#include "stdexcept"

namespace libmodbus_cpp {

enum class Mode {
    Master,
    Slave
};

enum class Type {
    TCP,
    RTU
};

enum class DataType {
    Coil,
    DiscreteInput,
    HoldingRegister,
    InputRegister
};


// exceptions ==============================================================
using RemoteRWError = std::runtime_error;

class RemoteReadError : public RemoteRWError {
public:
    RemoteReadError(const std::string &msg) : RemoteRWError(msg) {}
};

class RemoteWriteError : public RemoteRWError {
public:
    RemoteWriteError(const std::string &msg) : RemoteRWError(msg) {}
};

using LocalRWError = std::logic_error;

class LocalReadError : public LocalRWError {
public:
    LocalReadError(const std::string &msg) : LocalRWError(msg) {}
};

class LocalWriteError : public LocalRWError {
public:
    LocalWriteError(const std::string &msg) : LocalRWError(msg) {}
};
// =========================================================================

}

#endif // DEFS_H
