#include <QMap>
#include <QDebug>
#include <modbus/modbus-private.h>
#include "modbus/modbus-rtu-private.h"
#include <libmodbus_cpp/slave_rtu_backend.h>
#include <libmodbus_cpp/global.h>

QSerialPort *libmodbus_cpp::SlaveRtuBackend::m_staticPort = nullptr;

libmodbus_cpp::SlaveRtuBackend::SlaveRtuBackend()
    : m_verbose(libmodbus_cpp::isVerbose())
{
    if (m_verbose) {
        qDebug() << "modbus slave rtu created";
    }
}

libmodbus_cpp::SlaveRtuBackend::~SlaveRtuBackend()
{
    try {
        if (m_verbose) {
            qDebug() << "modbus slave rtu DTOR";
        }
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
    if (m_verbose) {
        qDebug() << "init";
    }

    modbus_t *ctx = modbus_new_rtu(device, baud, (char)parity, (int)dataBits, (int)stopBits);
    if (!ctx) {
        qWarning() << "ctx craete error";
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
        qInfo() << "libmodbus debug on";
    }
}

bool libmodbus_cpp::SlaveRtuBackend::doStartListen()
{
    if (m_verbose) {
        qDebug() << "start listen";
    }
    bool res = openConnection();
    if (res) {
#ifdef _WIN32
        // Close opened handle to get opened port in WIN by QSerialPort
        modbus_rtu_t *ctx_rtu = reinterpret_cast<modbus_rtu_t *>(getCtx()->backend_data);
        CloseHandle(ctx_rtu->w_ser.fd);
#endif
        if (!m_serialPort.open(QIODevice::ReadWrite)) {
            qWarning() << "can't open uart port!";
        } else {
            qInfo() << "uart port opened";
        }
        if (!connect(&m_serialPort, &QSerialPort::readyRead, this, &SlaveRtuBackend::slot_readFromPort)) {
            qWarning() << "can't connect to read port signal!";
        }
    }

    if (m_verbose) {
        qDebug() << "start listen res =" << res;
    }

    return res;
}

void libmodbus_cpp::SlaveRtuBackend::doStopListen()
{
    m_serialPort.close();
}

void libmodbus_cpp::SlaveRtuBackend::slot_readFromPort()
{
    if (m_verbose) {
        qDebug() << "Read from port" << m_serialPort.portName();
    }
    m_staticPort = &m_serialPort;
    std::array<uint8_t, MODBUS_RTU_MAX_ADU_LENGTH> buf;
    int messageLength = modbus_receive(getCtx(), buf.data());
    if (messageLength > 0) {
        if (m_verbose)
            qDebug() << "received:" << QByteArray(reinterpret_cast<const char*>(buf.data()), messageLength);
        processHooks(buf.data(), messageLength, HookTime::Preprocessing);
        modbus_reply(getCtx(), buf.data(), messageLength, getMap());
    } else if (messageLength == -1) {
        if (m_verbose)
            qDebug() << modbus_strerror(errno);
    }
    m_staticPort = nullptr;
}

int libmodbus_cpp::SlaveRtuBackend::customSelect(modbus_t *ctx, fd_set *rset, timeval *tv, int msg_length)
{
    if (libmodbus_cpp::isVerbose()) {
        qDebug() << "in custom select = ";
    }

    Q_UNUSED(ctx);
    Q_UNUSED(rset);
    Q_UNUSED(tv);
    Q_UNUSED(msg_length);
    //TODO 1: seems it will be buzy wait. Recheck!
    return m_staticPort->bytesAvailable();
}

ssize_t libmodbus_cpp::SlaveRtuBackend::customRecv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    if (libmodbus_cpp::isVerbose()) {
        qDebug() << "in custom receive";
    }

    Q_UNUSED(ctx);
    return m_staticPort->read(reinterpret_cast<char*>(rsp), rsp_length);
}


ssize_t libmodbus_cpp::SlaveRtuBackend::customSend(modbus_t *ctx, const uint8_t *rsp, int rsp_length)
{
    if (libmodbus_cpp::isVerbose()) {
        qDebug() << "in custom send";
    }

    Q_UNUSED(ctx);
    return m_staticPort->write(reinterpret_cast<const char*>(rsp), rsp_length);
}
