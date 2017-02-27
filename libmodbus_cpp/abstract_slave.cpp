#include <cassert>
#include <libmodbus_cpp/abstract_slave.h>

libmodbus_cpp::AbstractSlave::AbstractSlave(AbstractSlaveBackend *backend) :
    m_backend(backend)
{
    assert(m_backend && "Backend must be initialized in slave");
}

bool libmodbus_cpp::AbstractSlave::initMap(int holdingBitsCount, int inputBitsCount, int holdingRegistersCount, int inputRegistersCount)
{
    return getBackend()->initMap(holdingBitsCount, inputBitsCount, holdingRegistersCount, inputRegistersCount);
}

bool libmodbus_cpp::AbstractSlave::setAddress(uint8_t address)
{
    return (modbus_set_slave(getBackend()->getCtx(), address) != -1);
}

bool libmodbus_cpp::AbstractSlave::setDefaultAddress()
{
    return setAddress(MODBUS_TCP_SLAVE);
}

void libmodbus_cpp::AbstractSlave::addHook(libmodbus_cpp::FunctionCode funcCode, libmodbus_cpp::Address address, libmodbus_cpp::HookFunction func)
{
    getBackend()->addPreMessageHook(funcCode, address, func);
}

void libmodbus_cpp::AbstractSlave::addPreMessageHook(libmodbus_cpp::FunctionCode funcCode, libmodbus_cpp::Address address, libmodbus_cpp::HookFunction func)
{
    getBackend()->addPreMessageHook(funcCode, address, func);
}

void libmodbus_cpp::AbstractSlave::addPostMessageHook(libmodbus_cpp::FunctionCode funcCode, libmodbus_cpp::Address address, libmodbus_cpp::HookFunction func)
{
    getBackend()->addPostMessageHook(funcCode, address, func);
}

void libmodbus_cpp::AbstractSlave::registerHook(libmodbus_cpp::DataType type, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::Address rangeSize, libmodbus_cpp::HookTime hookTime, libmodbus_cpp::UniHookFunction func)
{
    getBackend()->addUniHook(type, AccessMode::Read, rangeBaseAddress, rangeSize, hookTime, func);
    getBackend()->addUniHook(type, AccessMode::Write, rangeBaseAddress, rangeSize, hookTime, func);
}

void libmodbus_cpp::AbstractSlave::registerHook(DataType type, libmodbus_cpp::AccessMode accessMode, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::Address rangeSize, libmodbus_cpp::HookTime hookTime, libmodbus_cpp::UniHookFunction func)
{
    getBackend()->addUniHook(type, accessMode, rangeBaseAddress, rangeSize, hookTime, func);
}

void libmodbus_cpp::AbstractSlave::registerReadHookOnRange(libmodbus_cpp::DataType type, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::Address rangeSize, libmodbus_cpp::UniHookFunction func, libmodbus_cpp::HookTime hookTime)
{
    registerHook(type, libmodbus_cpp::AccessMode::Read, rangeBaseAddress, rangeSize, hookTime, func);
}

void libmodbus_cpp::AbstractSlave::registerWriteHookOnRange(libmodbus_cpp::DataType type, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::Address rangeSize, libmodbus_cpp::UniHookFunction func, libmodbus_cpp::HookTime hookTime)
{
    registerHook(type, libmodbus_cpp::AccessMode::Write, rangeBaseAddress, rangeSize, hookTime, func);
}

void libmodbus_cpp::AbstractSlave::registerReadHook(DataType type, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::UniHookFunction func, libmodbus_cpp::HookTime hookTime)
{
    registerReadHookOnRange(type, rangeBaseAddress, 1, func, hookTime);
}

void libmodbus_cpp::AbstractSlave::registerWriteHook(DataType type, libmodbus_cpp::Address rangeBaseAddress, libmodbus_cpp::UniHookFunction func, libmodbus_cpp::HookTime hookTime)
{
        registerWriteHookOnRange(type, rangeBaseAddress, 1, func, hookTime);
}

bool libmodbus_cpp::AbstractSlave::startListen()
{
    return getBackend()->startListen();
}

void libmodbus_cpp::AbstractSlave::stopListen()
{
    getBackend()->stopListen();
}

void libmodbus_cpp::AbstractSlave::setValueToCoil(uint16_t address, bool value) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalWriteError("map was not inited");
    if (map->nb_bits <= address)
        throw LocalWriteError("wrong address");
    setValueToTable(map->tab_bits, address, value);
}

bool libmodbus_cpp::AbstractSlave::getValueFromCoil(uint16_t address) {
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalReadError("map was not inited");
    if (map->nb_bits <= address)
        throw LocalReadError("wrong address");
    return getValueFromTable<bool>(map->tab_bits, address);
}

void libmodbus_cpp::AbstractSlave::setValueToDiscreteInput(uint16_t address, bool value)
{
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalWriteError("map was not inited");
    if (map->nb_input_bits <= address)
        throw LocalWriteError("wrong address");
    setValueToTable(map->tab_input_bits, address, value);
}

bool libmodbus_cpp::AbstractSlave::getValueFromDiscreteInput(uint16_t address)
{
    modbus_mapping_t * map = getBackend()->getMap();
    if (!map)
        throw LocalReadError("map was not inited");
    if (map->nb_input_bits <= address)
        throw LocalReadError("wrong address");
    return getValueFromTable<bool>(map->tab_input_bits, address);
}
