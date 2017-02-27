#ifndef LIBMODBUS_CPP_BACKEND_H
#define LIBMODBUS_CPP_BACKEND_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMap>
#include <functional>
#include <cstring>
#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <modbus/modbus-tcp.h>
#include "defs.h"

namespace libmodbus_cpp {

template<typename RegType, typename ValueType>
inline RegType extractRegisterFromValue(int idx, const ValueType &value) {
    return (RegType)(value >> (8 * sizeof(RegType) * idx));
}

template<typename RegType, typename ValueType>
inline RegType extractRegisterFromValue_unsafe(int idx, const ValueType &value) {
    return *(static_cast<RegType*>(&value) + idx);
}

template<typename RegType, typename ValueType>
inline void insertRegisterIntoValue(int idx, ValueType &value, RegType reg) {
    RegType mask = -1; // all ones
    value = value & ~(mask << idx) | (reg << idx);
}

template<typename RegType, typename ValueType>
inline void insertRegisterIntoValue_unsafe(int idx, ValueType &value, RegType reg) {
    *(static_cast<RegType*>(&value) + idx) = reg;
}


class AbstractBackend
{
    modbus_t *m_ctx = Q_NULLPTR;
    ByteOrder targetByteOrder = ByteOrder::LittleEndian;
    ByteOrder systemByteOrder = checkSystemByteOrder();

protected:
    AbstractBackend();

public:
    virtual ~AbstractBackend();

    void setCtx(modbus_t *ctx);

    inline modbus_t *getCtx() {
        return m_ctx;
    }

    bool openConnection();
    void closeConnection();

    bool doesSystemByteOrderMatchTarget() const;

private:
    static ByteOrder checkSystemByteOrder();
};

class AbstractSlaveBackendPrivate;
class AbstractSlaveBackend : public AbstractBackend
{
protected:
    AbstractSlaveBackend();

    void processHooks(const uint8_t *req, int req_length, HookTime hookTime);

    virtual bool doStartListen() = 0;
    virtual void doStopListen() = 0;

public:
    ~AbstractSlaveBackend() override;

    modbus_mapping_t *getMap();

    bool initMap(int holdingBitsCount, int inputBitsCount, int holdingRegistersCount, int inputRegistersCount);
    bool initRegisterMap(int holdingRegistersCount, int inputRegistersCount);

    bool startListen();
    void stopListen();

    void addUniHook(DataType type, AccessMode accessMode, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func);
    void addPreMessageHook(FunctionCode funcCode, Address address, HookFunction func);
    void addPostMessageHook(FunctionCode funcCode, Address address, HookFunction func);
private:
    friend class AbstractSlaveBackendPrivate;
    QScopedPointer<AbstractSlaveBackendPrivate> ad_ptr;
    AbstractSlaveBackendPrivate* d_ptr;
};

}

#endif // LIBMODBUS_CPP_BACKEND_H
