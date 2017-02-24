#include <libmodbus_cpp/factory.h>
#include <libmodbus_cpp/master_tcp.h>
#include <libmodbus_cpp/slave_tcp.h>
#ifdef USE_QT5
#include <libmodbus_cpp/master_rtu.h>
#include <libmodbus_cpp/slave_rtu.h>
#endif

std::unique_ptr<libmodbus_cpp::AbstractMaster> libmodbus_cpp::Factory::createTcpMaster(const char *address, int port)
{
    std::unique_ptr<MasterTcpBackend> b(new MasterTcpBackend());
    b->init(address, port);
    return std::unique_ptr<AbstractMaster>(new MasterTcp(b.release()));
}

std::unique_ptr<libmodbus_cpp::AbstractSlave> libmodbus_cpp::Factory::createTcpSlave(const char *address, int port)
{
    std::unique_ptr<SlaveTcpBackend> b(new SlaveTcpBackend());
    b->init(address, port);
    return std::unique_ptr<AbstractSlave>(new SlaveTcp(b.release()));
}

#ifdef USE_QT5
std::unique_ptr<libmodbus_cpp::AbstractMaster> libmodbus_cpp::Factory::createRtuMaster(const char *device, int baud, libmodbus_cpp::Parity parity, DataBits dataBits, StopBits stopBits)
{
    std::unique_ptr<MasterRtuBackend> b(new MasterRtuBackend());
    b->init(device, baud, parity, dataBits, stopBits);
    return std::unique_ptr<AbstractMaster>(new MasterRtu(b.release()));
}

std::unique_ptr<libmodbus_cpp::AbstractSlave> libmodbus_cpp::Factory::createRtuSlave(const char *device, int baud, libmodbus_cpp::Parity parity, DataBits dataBits, StopBits stopBits)
{
    std::unique_ptr<SlaveRtuBackend> b(new SlaveRtuBackend());
    b->init(device, baud, parity, dataBits, stopBits);
    return std::unique_ptr<AbstractSlave>(new SlaveRtu(b.release()));
}
#endif
