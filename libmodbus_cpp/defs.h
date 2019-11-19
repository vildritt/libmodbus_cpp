#ifndef LIBMODBUS_CPP_DEFS_H
#define LIBMODBUS_CPP_DEFS_H

#include <stdexcept>
#include <functional>

#include <modbus/modbus.h>

#include <QByteArray>

namespace libmodbus_cpp {

enum class ByteOrder {
    LittleEndian,
    BigEndian
};

enum class Mode {
    Master,
    Slave
};

enum class Type {
    TCP,
    RTU
};

enum class Parity : char {
    None = 'N',
    Even = 'E',
    Odd = 'O'
};

enum class DataBits {
    b5 = 5,
    b6 = 6,
    b7 = 7,
    b8 = 8
};

enum class StopBits {
    b1 = 1,
    b2 = 2
};

enum class DataType {
    Coil,
    DiscreteInput,
    HoldingRegister,
    InputRegister
};

struct RawResult {
    uint8_t address;
    uint8_t functionCode;
    QByteArray data;

    bool isError() {
        return (functionCode & 0x80);
    }
};

enum class AccessMode {
    Read,
    Write
};

enum class HookTime {
    Preprocessing,
    Postprocessing
};

// hooks ===================================================================
using FunctionCode = uint8_t;
using Address = uint16_t;

struct AddressRange {
    AddressRange()
        : from(1), to(0)
    {}

    AddressRange(Address from, Address to)
        : from(from), to(to)
    {}

    Address from;
    Address to;

    bool isValid() const {
        return (to >= from);
    }

    AddressRange& shift(int da) {
        from += da;
        to   += da;
        return *this;
    }

    static AddressRange intersection(const AddressRange& A, const AddressRange& B) {

        if ((B.from > A.to) || (A.from > B.to)) {
          return AddressRange();
        } else {
            return AddressRange(
                        (A.from < B.from) ? B.from : A.from,
                        (A.to   > B.to  ) ? B.to   : A.to);
        }
    }

    bool intersectsWith(const AddressRange& inst) const {
        return intersection(*this, inst).isValid();
    }

    static AddressRange fromSizedRange(Address from, Address length) {
        AddressRange res;
        res.from = from;
        res.to = (Address)((int)from + (int)length - 1);
        return res;
    }
};

struct UniHookInfo {
    FunctionCode function;

    DataType type;
    AccessMode accessMode;
    HookTime hookTime;

    Address rangeBaseAddress;
    Address rangeSize;

    AddressRange range;
};

using HookFunction = std::function<void(void)>;
using UniHookFunction = std::function<void(const UniHookInfo* info)>;

// exceptions ==============================================================
using Exception = std::runtime_error;
using RemoteRWError = Exception;

class RemoteReadError : public RemoteRWError {
public:
    RemoteReadError(const std::string &msg) : RemoteRWError(msg) {}
};

class RemoteWriteError : public RemoteRWError {
public:
    RemoteWriteError(const std::string &msg) : RemoteRWError(msg) {}
};

class ConnectionError : public RemoteRWError {
public:
    ConnectionError(const std::string &msg) : RemoteRWError(msg) {}
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

#endif // LIBMODBUS_CPP_DEFS_H

