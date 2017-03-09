#include <QMap>
#include <QDebug>
#include <QThread>
#include <QTime>
#include <QCoreApplication>
#include <modbus/modbus-private.h>
#include "modbus/modbus-rtu-private.h"
#include <libmodbus_cpp/slave_rtu_backend.h>
#include <libmodbus_cpp/global.h>
#include "logger.h"


QSerialPort *libmodbus_cpp::SlaveRtuBackend::m_staticPort = nullptr;


#define LDOM_SRTU "[modbus.slave.rtu]"
#define LDOM_IO   "[modbus.slave.rtu.io]"
#define LDOM_EVT  "[modbus.slave.rtu.io.evt]"
#define LDOM_PKT  "[modbus.slave.rtu.io.pkt]"


libmodbus_cpp::SlaveRtuBackend::SlaveRtuBackend()
    : m_verbose(libmodbus_cpp::isVerbose())
{
    LMB_DLOG(LDOM_SRTU, "ctor");
}


libmodbus_cpp::SlaveRtuBackend::~SlaveRtuBackend()
{
    try {
        LMB_DLOG(LDOM_SRTU, "dtor");
        auto ctx = getCtx();
        if (ctx) {
            ctx->backend = m_originalBackend; // for normal deinit by libmodbus
        }
        stopListen();
    } catch(...) {

    }

}


void libmodbus_cpp::SlaveRtuBackend::init(const char *device, int baud, libmodbus_cpp::Parity parity, libmodbus_cpp::DataBits dataBits, libmodbus_cpp::StopBits stopBits)
{
    LMB_DLOG(LDOM_SRTU, "init connection with: " << device << "BR:" << baud);

    modbus_t *ctx = modbus_new_rtu(device, baud, (char)parity, (int)dataBits, (int)stopBits);
    if (!ctx) {
        LMB_WLOG(LDOM_SRTU, "ctx create error");
        throw Exception(std::string("Failed to create RTU context: ") + modbus_strerror(errno));
    }
    setCtx(ctx);

    m_serialPort.setPortName(device);
    m_serialPort.setBaudRate(baud);
    static QMap<Parity, QSerialPort::Parity> parityConvertionMap = {
        { Parity::None, QSerialPort::NoParity },
        { Parity::Even, QSerialPort::EvenParity },
        { Parity::Odd, QSerialPort::OddParity }
    };
    m_serialPort.setParity(parityConvertionMap[parity]);
    m_serialPort.setDataBits((QSerialPort::DataBits)dataBits);
    m_serialPort.setStopBits((QSerialPort::StopBits)stopBits);

    m_originalBackend = getCtx()->backend;
    m_customBackend.reset(new modbus_backend_t);
    std::memcpy(m_customBackend.data(), m_originalBackend, sizeof(*m_customBackend));
    m_customBackend->select = customSelect;
    m_customBackend->recv = customRecv;
#ifdef _WIN32
    m_customBackend->send = customSend;
#endif
    getCtx()->backend = m_customBackend.data();
    getCtx()->debug = m_verbose ? 1 : 0;
    if (getCtx()->debug) {
        LMB_ILOG(LDOM_SRTU, "verbose mode");
    }
}


bool libmodbus_cpp::SlaveRtuBackend::doStartListen()
{
    LMB_DLOG(LDOM_SRTU, "start server");
    bool res = openConnection();
    if (res) {
#ifdef _WIN32
        // Close opened handle to get opened port in WIN by QSerialPort
        modbus_rtu_t *ctx_rtu = reinterpret_cast<modbus_rtu_t *>(getCtx()->backend_data);
        CloseHandle(ctx_rtu->w_ser.fd);
#endif
        if (!m_serialPort.open(QIODevice::ReadWrite)) {
            LMB_WLOG(LDOM_SRTU, "can't open uart port = " << m_serialPort.portName());
        } else {
            LMB_DLOG(LDOM_SRTU, "uart opened");
        }
        if (!connect(&m_serialPort, &QSerialPort::readyRead, this, &SlaveRtuBackend::slot_readFromPort)) {
            LMB_WLOG(LDOM_SRTU, "can't connect to read port signal!");
        }
    }

    LMB_DLOG(LDOM_SRTU, "server start result = " << res);

    return res;
}


void libmodbus_cpp::SlaveRtuBackend::doStopListen()
{
    m_serialPort.close();
}


void libmodbus_cpp::SlaveRtuBackend::slot_readFromPort()
{
    LMB_DLOG(LDOM_EVT, "new data in port");

    static bool inProcess = false;

    if (inProcess) {
        return;
    }

    inProcess = true;

    while (m_serialPort.bytesAvailable() > 0) {
        LMB_DLOG(LDOM_PKT, "try read rest of data size = " << m_serialPort.bytesAvailable());
        m_staticPort = &m_serialPort;
        std::array<uint8_t, MODBUS_RTU_MAX_ADU_LENGTH> buf;
        int messageLength = modbus_receive(getCtx(), buf.data());
        if (messageLength > 0) {
            LMB_DLOG(LDOM_PKT, "received packet: " << BUF2HEX(buf.data(), messageLength));
            processHooks(buf.data(), messageLength, HookTime::Preprocessing);
            modbus_reply(getCtx(), buf.data(), messageLength, getMap());
            processHooks(buf.data(), messageLength, HookTime::Postprocessing);
        } else if (messageLength == -1) {
            LMB_WLOG(LDOM_PKT, modbus_strerror(errno));
        }

        m_staticPort = nullptr;
    }
    inProcess = false;
}


int libmodbus_cpp::SlaveRtuBackend::customSelect(modbus_t *ctx, fd_set *rset, timeval *tv, int msg_length)
{
    const double timeous_ms =
            (tv)
            ? (tv->tv_sec * 1000.0 + tv->tv_usec / 1000.0)
            : 0.0;

    LMB_DGLOG(LDOM_IO,
             "select. Wait for " << msg_length << " bytes. " <<
             "Avail" << m_staticPort->bytesAvailable() <<
             "TO = " << (tv ? timeous_ms : -1.0));

    Q_UNUSED(ctx);
    Q_UNUSED(rset);


    if (!m_staticPort) {
        LMB_WGLOG(LDOM_IO, "SP is nil");
        return -1;
    }

    QTime timeoutTimer;
    timeoutTimer.start();

    qint64 prevAvail = m_staticPort->bytesAvailable();
    while(m_staticPort->bytesAvailable() < msg_length) {
        if (tv) {
            if (timeoutTimer.elapsed() > timeous_ms)  {
                LMB_WGLOG(LDOM_IO, "Timeout! Elapsed[ms] = " << timeoutTimer.elapsed());
                return 0; // no descriptors signaled
            }
        }

        QEventLoop el;
        static const int pollInterval_ms = 50;
        el.processEvents(QEventLoop::AllEvents, pollInterval_ms);

        if (!m_staticPort) {
            LMB_WGLOG(LDOM_IO, "SP is nil");
            return -1;
        }

        const qint64 avail = m_staticPort->bytesAvailable();
        if (avail != prevAvail) {
            LMB_DGLOG(LDOM_IO, "avail = " << m_staticPort->bytesAvailable());
            prevAvail = avail;
        }
    }

    return 1; // count if signaled descriptors
}


ssize_t libmodbus_cpp::SlaveRtuBackend::customRecv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    Q_UNUSED(ctx);

    const int readedCount = m_staticPort->read(reinterpret_cast<char*>(rsp), rsp_length);

    LMB_DGLOG(LDOM_IO, "recv size = " << readedCount);
    if (readedCount > 0) {
        LMB_DGLOG(LDOM_IO, "recv data = " << BUF2HEX(rsp, readedCount));
    }

    return readedCount;
}


ssize_t libmodbus_cpp::SlaveRtuBackend::customSend(modbus_t *ctx, const uint8_t *rsp, int rsp_length)
{
    Q_UNUSED(ctx);

    LMB_DGLOG(LDOM_IO, "send size = " << rsp_length);
    if (rsp_length > 0) {
        LMB_DGLOG(LDOM_IO, "send data = " << BUF2HEX(rsp, rsp_length));
    }

    const int writedCount = m_staticPort->write(reinterpret_cast<const char*>(rsp), rsp_length);

    LMB_DGLOG(LDOM_IO, "sent = " << writedCount);

    return writedCount;
}
