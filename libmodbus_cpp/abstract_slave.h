#ifndef LIBMODBUS_CPP_ABSTRACTSLAVE_H
#define LIBMODBUS_CPP_ABSTRACTSLAVE_H

#include <QScopedPointer>
#include <stdexcept>
#include <iterator>
#include "backend.h"
#include "defs.h"

namespace libmodbus_cpp {

void reverseBytes(char* data, unsigned int size);
void reverseBytesPairs(char* data, unsigned int size);
void mb_memcopy(const char* source, unsigned int size, char* distance, const ByteOrder target);

class AbstractSlave
{
    QScopedPointer<AbstractSlaveBackend> m_backend;

protected:
    AbstractSlave(AbstractSlaveBackend *backend);
    virtual inline AbstractSlaveBackend *getBackend() {
        return m_backend.data();
    }

public:
    virtual ~AbstractSlave() {}

    bool initMap(int holdingBitsCount, int inputBitsCount, int holdingRegistersCount, int inputRegistersCount);
    bool setAddress(uint8_t address);
    bool setDefaultAddress();

    //[[deprecated("use addPreMessageHook insted")]]
    void addHook(FunctionCode funcCode, Address address, HookFunction func);
    void addPreMessageHook(FunctionCode funcCode, Address address, HookFunction func);
    void addPostMessageHook(FunctionCode funcCode, Address address, HookFunction func);

    void registerHook(DataType type, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func);
    void registerHook(DataType type, AccessMode accessMode, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func);
    void registerReadHook (DataType type, Address rangeBaseAddress, UniHookFunction func, HookTime hookTime = HookTime::Preprocessing);
    void registerWriteHook(DataType type, Address rangeBaseAddress, UniHookFunction func, HookTime hookTime = HookTime::Postprocessing);
    void registerReadHookOnRange (DataType type, Address rangeBaseAddress, Address rangeSize, UniHookFunction func, HookTime hookTime = HookTime::Preprocessing);
    void registerWriteHookOnRange(DataType type, Address rangeBaseAddress, Address rangeSize, UniHookFunction func, HookTime hookTime = HookTime::Postprocessing);

    bool startListen();
    void stopListen();

    void setValueToCoil(uint16_t address, bool value);
    bool getValueFromCoil(uint16_t address);

    void setValueToDiscreteInput(uint16_t address, bool value);
    bool getValueFromDiscreteInput(uint16_t address);

    template<typename ValueType>
    void setValueToHoldingRegister(uint16_t address, ValueType value);
    template<typename ValueType>
    ValueType getValueFromHoldingRegister(uint16_t address);

    template<typename ValueType>
    void setValueToInputRegister(uint16_t address, ValueType value);
    template<typename ValueType>
    ValueType getValueFromInputRegister(uint16_t address);

private:

    template<typename ValueType, typename TableType>
    void setValueToTable(TableType *table, uint16_t address, const ValueType &value) {
        int offset = sizeof(TableType) * address;
        if (getBackend()->doesSystemNativeByteOrderMatchTarget())
            std::memcpy(reinterpret_cast<uint8_t*>(table) + offset, &value, sizeof(ValueType));
        else {
            const uint8_t *valueAsArray = reinterpret_cast<const uint8_t*>(&value);
            int size = sizeof(ValueType);
            for (int i = size - 1, j = 0; i >= 0; --i, ++j)
                *(reinterpret_cast<uint8_t*>(table) + offset + j) = valueAsArray[i];
        }
    }

    template<typename ValueType, typename TableType>
    ValueType getValueFromTable(TableType *table, uint16_t address) {
        ValueType res(0);
        int offset = sizeof(TableType) * address;
        if (getBackend()->doesSystemNativeByteOrderMatchTarget())
            std::memcpy(&res, reinterpret_cast<uint8_t*>(table) + offset, sizeof(ValueType));
        else {
            uint8_t *resAsArray = reinterpret_cast<uint8_t*>(&res);
            int size = sizeof(ValueType);
            for (int i = size - 1, j = 0; i >= 0; --i, ++j)
                resAsArray[i] = *(reinterpret_cast<uint8_t*>(table) + offset + j);
        }
        return res;
    }
};

template<typename ValueType>
void AbstractSlave::setValueToHoldingRegister(uint16_t address, ValueType value) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalWriteError("map was not inited");
    if (map->nb_registers <= address)
        throw LocalWriteError("wrong address");
    setValueToTable(map->tab_registers, address, value);
}

template<typename ValueType>
ValueType AbstractSlave::getValueFromHoldingRegister(uint16_t address) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalReadError("map was not inited");
    if (map->nb_registers <= address)
        throw LocalReadError("wrong address");
    return getValueFromTable<ValueType>(map->tab_registers, address);
}

template<typename ValueType>
void AbstractSlave::setValueToInputRegister(uint16_t address, ValueType value) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalWriteError("map was not inited");
    if (map->nb_input_registers <= address)
        throw LocalWriteError("wrong address");
    setValueToTable(map->tab_input_registers, address, value);
}

template<typename ValueType>
ValueType AbstractSlave::getValueFromInputRegister(uint16_t address) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalReadError("map was not inited");
    if (map->nb_input_registers <= address)
        throw LocalReadError("wrong address");
    return getValueFromTable<ValueType>(map->tab_input_registers, address);
}

}

#endif // LIBMODBUS_CPP_ABSTRACTSLAVE_H
