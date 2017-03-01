#ifndef LIBMODBUS_CPP_ABSTRACTSLAVE_H
#define LIBMODBUS_CPP_ABSTRACTSLAVE_H

#include <QScopedPointer>
#include <stdexcept>
#include <iterator>
#include "backend.h"
#include "defs.h"
#include "mapping_wrapper.h"

namespace libmodbus_cpp {

// modbus data model impl io with app memory
void registerMemoryCopy(const void *source, unsigned int size, void *distance, const ByteOrder target);
void setModbusBit(uint8_t *table, Address address, bool value);
bool getModbusBit(uint8_t *table, uint16_t address);


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

    /// setup

    bool initMap(int holdingBitsCount, int inputBitsCount, int holdingRegistersCount, int inputRegistersCount);
    bool setAddress(uint8_t address);
    bool setDefaultAddress();

    /// hooks

    //[[deprecated("use addPreMessageHook insted or register* methods")]]
    void addHook(FunctionCode funcCode, Address address, HookFunction func);
    void addPreMessageHook(FunctionCode funcCode, Address address, HookFunction func);
    void addPostMessageHook(FunctionCode funcCode, Address address, HookFunction func);

    void registerHook(DataType type, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func);
    void registerHook(DataType type, AccessMode accessMode, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func);
    void registerReadHook (DataType type, Address rangeBaseAddress, UniHookFunction func, HookTime hookTime = HookTime::Preprocessing);
    void registerWriteHook(DataType type, Address rangeBaseAddress, UniHookFunction func, HookTime hookTime = HookTime::Postprocessing);
    void registerReadHookOnRange (DataType type, Address rangeBaseAddress, Address rangeSize, UniHookFunction func, HookTime hookTime = HookTime::Preprocessing);
    void registerWriteHookOnRange(DataType type, Address rangeBaseAddress, Address rangeSize, UniHookFunction func, HookTime hookTime = HookTime::Postprocessing);

    /// activation

    bool startListen();
    void stopListen();
    void setTargetByteOrder(ByteOrder byteOrder);

    /// data access

    // bits

    template<DataType dataType>
    void setBit(Address address, bool value) {
        const auto m = getBackend()->getMapper<dataType>(address);
        setModbusBit(m.bitTable(), address, value);
    }

    template<DataType dataType>
    bool getBit(Address address) {
        const auto m = getBackend()->getMapper<dataType>(address);
        return getModbusBit(m.bitTable(), address);
    }

    // registers

    template<typename ValueType, DataType dataType>
    void      setValue(Address address, ValueType value) {
        setValueToRegs(getBackend()->getMapper<dataType>(address).regTable(), address, value);
    }

    template<typename ValueType, DataType dataType>
    ValueType getValue(Address address) {
        return getValueFromRegs<ValueType>(getBackend()->getMapper<dataType>(address).regTable(), address);
    }

    // NOTE: old intf but also it's needed to ceate all template funcs!

    void setValueToCoil(Address address, bool value);
    bool getValueFromCoil(Address address);

    void setValueToDiscreteInput(Address address, bool value);
    bool getValueFromDiscreteInput(Address address);

    template<typename ValueType>
    void setValueToHoldingRegister(Address address, ValueType value) {
        setValue<ValueType, DataType::HoldingRegister>(address, value);
    }
    template<typename ValueType>
    ValueType getValueFromHoldingRegister(Address address) {
        return getValue<ValueType, DataType::HoldingRegister>(address);
    }
    template<typename ValueType>
    void setValueToInputRegister(Address address, ValueType value) {
        setValue<ValueType, DataType::InputRegister>(address, value);
    }
    template<typename ValueType>
    ValueType getValueFromInputRegister(Address address) {
        return getValue<ValueType, DataType::InputRegister>(address);
    }

    template<DataType DT>
    void fillWith(Address address, uint8_t value, int byteSize) {
        const auto m = getBackend()->getMapper<DT>(address);
        uint8_t* d = (uint8_t*)(m.table());
        memset(d, value, byteSize);
    }

private:

    template<typename ValueType>
    void setValueToRegs(uint16_t *table, uint16_t address, const ValueType &value) {
        registerMemoryCopy(&value, sizeof(ValueType), table + address, getBackend()->getTargetByteOrder());
    }

    template<typename ValueType>
    ValueType getValueFromRegs(uint16_t *table, uint16_t address) {
        ValueType res;
        registerMemoryCopy(table + address, sizeof(ValueType), &res, getBackend()->getTargetByteOrder());
        return res;
    }

};

} // ns


#endif // LIBMODBUS_CPP_ABSTRACTSLAVE_H
