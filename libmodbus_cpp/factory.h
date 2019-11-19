#ifndef LIBMODBUS_CPP_FACTORY_H
#define LIBMODBUS_CPP_FACTORY_H

#include "defs.h"
#include <memory>

namespace libmodbus_cpp {

class AbstractSlave;
class AbstractMaster;

class Factory
{
public:
    static std::unique_ptr<AbstractMaster> createTcpMaster(const char *address, int port);
    static std::unique_ptr<AbstractSlave> createTcpSlave(const char *address, int port);
#ifdef USE_QT5
    static std::unique_ptr<AbstractMaster> createRtuMaster(const char *device, int baud, Parity parity = Parity::None, DataBits dataBits = DataBits::b8, StopBits stopBits = StopBits::b1);
    static std::unique_ptr<AbstractSlave> createRtuSlave(const char *device, int baud, Parity parity = Parity::None, DataBits dataBits = DataBits::b8, StopBits stopBits = StopBits::b1);
#endif
};

}

#endif // LIBMODBUS_CPP_FACTORY_H
