#ifndef LIBMODBUS_CPP_MAPPING_WRAPPER_H_GUARD
#define LIBMODBUS_CPP_MAPPING_WRAPPER_H_GUARD

#include "defs.h"


namespace libmodbus_cpp {


template<DataType T>
struct MappingWrapper {
};


#define LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION(type, name) \
    template<> \
    struct MappingWrapper<type> { \
        modbus_mapping_t* map; \
        MappingWrapper(modbus_mapping_t* map) : map(map) {} \
        int   count() const  { \
            return map->nb_ ## name; \
        } \
        int   offset() const { \
            return map->offset_ ## name; \
        } \
        void *table() const  { \
            return map->tab_ ## name; \
        } \
        uint16_t *regTable() const  { \
            return reinterpret_cast<uint16_t *>(map->tab_ ## name); \
        } \
        uint8_t *bitTable() const  { \
            return reinterpret_cast<uint8_t *>(map->tab_ ## name); \
        } \
        bool  isAssigend() const { \
            return map != nullptr; \
        } \
    }


LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION(DataType::Coil, bits);
LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION(DataType::DiscreteInput, input_bits);
LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION(DataType::HoldingRegister, registers);
LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION(DataType::InputRegister, input_registers);

#undef LIBMODBUS_CPP_MAPPER_WRAPPER_DECLARATION

} // ns


#endif // LIBMODBUS_CPP_MAPPING_WRAPPER_H_GUARD

