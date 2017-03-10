#include <cassert>
#include <modbus/modbus-private.h>
#include <libmodbus_cpp/backend.h>
#include <QVector>
#include <QDebug>
#include <QTime>
#include <QEventLoop>
#include "logger.h"

#define LDOM_BK   "[modbus.slave.bk]"
#define LDOM_HOOK "[modbus.slave.bk.hook]"
#define LDOM_IO   "[modbus.slave.io]"


using namespace libmodbus_cpp;

AbstractBackend::AbstractBackend()
{
}

bool AbstractBackend::doesSystemNativeByteOrderMatchTarget() const
{
    return targetByteOrder == systemByteOrder;
}

void AbstractBackend::setTargetByteOrder(ByteOrder order)
{
    targetByteOrder = order;
}

ByteOrder AbstractBackend::getTargetByteOrder() const
{
    return targetByteOrder;
}

AbstractBackend::~AbstractBackend()
{
    if (m_ctx) {
        closeConnection();
        modbus_free(m_ctx);
    }
}

void AbstractBackend::setCtx(modbus_t *ctx)
{
    m_ctx = ctx;
}

bool AbstractBackend::openConnection()
{
    int errorCode = modbus_connect(getCtx());
    if (errorCode == 0)
        return true;
    else
        throw ConnectionError(modbus_strerror(errno));
}

void AbstractBackend::closeConnection()
{
    modbus_close(getCtx());
}

ByteOrder AbstractBackend::getSystemNativeByteOrder()
{
    union {
        unsigned short s;
        unsigned char c[2];
    } x { 0x0201 };
    return (x.c[1] > x.c[0]) ? ByteOrder::LittleEndian : ByteOrder::BigEndian;
}

using UniHookKey = int;

static UniHookKey uniHookKey(const DataType type, const AccessMode accessMode, const HookTime hookTime)
{
    const UniHookKey key =
            (static_cast<int>(type) << 16) +
            (static_cast<int>(accessMode) << 8) +
            (static_cast<int>(hookTime));
    LMB_DGLOG(LDOM_HOOK, "key([" << (int)type << (int)accessMode << (int)hookTime <<"]) = " << key);
    return key;
}

namespace  libmodbus_cpp {


class AbstractSlaveBackendPrivate {
public:
    using HooksByAddress = QMap<Address, HookFunction>;
    using HooksByFunctionCode = QMap<FunctionCode, HooksByAddress>;


    struct UniHookSetup {
        AddressRange range;
        UniHookFunction handler;

        bool isHit(const AddressRange& hitRng) const {
            return range.intersectsWith(hitRng);
        }
    };

    struct UniHooks {
        QVector<UniHookSetup> hooks;

        void add(Address rangeBaseAddress, Address rangeLength, UniHookFunction func) {
            UniHookSetup stp;
            stp.handler = func;
            stp.range = AddressRange::fromSizedRange(rangeBaseAddress, rangeLength);
            hooks.append(stp);
        }

        int count() const {
            return hooks.size();
        }

        void compile() {
            qSort(hooks.begin(), hooks.end(), [](const UniHookSetup &L, const UniHookSetup &R) -> bool{
                if (L.range.from == R.range.from) {
                    return L.range.to < R.range.to;
                } else {
                    return L.range.from < R.range.from;
                }
            });
        }

        void process(UniHookInfo& info) {
            info.range = AddressRange::fromSizedRange(info.rangeBaseAddress, info.rangeSize);

            //TODO 1: improove speed

            const int N = hooks.size();
            for(int i = 0; i < N; ++i) {
                const UniHookSetup& hookRange = hooks.at(i);
                if (hookRange.isHit(info.range)) {
                    hookRange.handler(&info);
                }
            }
        }
    };

    HooksByFunctionCode m_hooks;
    HooksByFunctionCode m_postMessageHooks;
    QMap<UniHookKey, UniHooks> m_uniHook;

    modbus_mapping_t *m_map = Q_NULLPTR;
    AbstractSlaveBackend* q;


    AbstractSlaveBackendPrivate(AbstractSlaveBackend* q) : q(q) {
        //stub
    }


    void tryProcessUniHook(UniHookInfo& info) {

        const UniHookKey key = uniHookKey(info.type, info.accessMode, info.hookTime);

        if (!m_uniHook.contains(key)) {
            return;
        }

        LMB_DGLOG(LDOM_HOOK, "call hook");
        m_uniHook[key].process(info);


    }

    void checkHookMap(const uint8_t *req, int req_length, const HooksByFunctionCode &oldHooks, HookTime hookTime) {

        Q_UNUSED(req_length);

#define GET_HDR_U16(ID)  ((req[offset + 1 + ((ID) << 1)] << 8) + req[offset + 2 + ((ID) << 1)])

        UniHookInfo info;

        const int offset = modbus_get_header_length(q->getCtx());

        info.function = req[offset];
        info.rangeBaseAddress = GET_HDR_U16(0);

        // check old style hooks
        {
            const auto &addrHooks = oldHooks[info.function];
            if (addrHooks.contains(info.rangeBaseAddress)) {
                addrHooks[info.rangeBaseAddress]();
            }
        }

        info.hookTime = hookTime;

        // special case
        if (info.function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            LMB_DGLOG(LDOM_HOOK, "R/W func");

            info.type = DataType::HoldingRegister;

            info.rangeBaseAddress = GET_HDR_U16(2);
            info.rangeSize = GET_HDR_U16(3);
            info.accessMode = AccessMode::Write;

            tryProcessUniHook(info);

            info.rangeBaseAddress = GET_HDR_U16(0);
            info.rangeSize = GET_HDR_U16(1);
            info.accessMode = AccessMode::Read;
            tryProcessUniHook(info);
            return;
        }

        switch(info.function) {
            case MODBUS_FC_READ_COILS              :
            case MODBUS_FC_READ_DISCRETE_INPUTS    :
            case MODBUS_FC_READ_HOLDING_REGISTERS  :
            case MODBUS_FC_READ_INPUT_REGISTERS    :
                info.accessMode = AccessMode::Read;
                break;
            case MODBUS_FC_WRITE_SINGLE_COIL       :
            case MODBUS_FC_WRITE_SINGLE_REGISTER   :
            case MODBUS_FC_WRITE_MULTIPLE_COILS    :
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            case MODBUS_FC_MASK_WRITE_REGISTER     :
                info.accessMode = AccessMode::Write;
                break;
            default:
                LMB_WGLOG(LDOM_HOOK, "unhookable function: " << info.function);
                return;
        }

        switch(info.function) {
            case MODBUS_FC_READ_COILS              :
            case MODBUS_FC_WRITE_MULTIPLE_COILS    :
            case MODBUS_FC_WRITE_SINGLE_COIL       :
                info.type = DataType::Coil;
                break;
            case MODBUS_FC_READ_DISCRETE_INPUTS    :
                info.type = DataType::DiscreteInput;
                break;
            case MODBUS_FC_READ_HOLDING_REGISTERS  :
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            case MODBUS_FC_WRITE_SINGLE_REGISTER   :
            case MODBUS_FC_MASK_WRITE_REGISTER     :
                info.type = DataType::HoldingRegister;
                break;
            case MODBUS_FC_READ_INPUT_REGISTERS    :
                info.type = DataType::InputRegister;
                break;
            default:
                LMB_WGLOG(LDOM_HOOK, "unhookable function: " << info.function);
                return;
        }
        switch(info.function) {
            case MODBUS_FC_READ_COILS              :
            case MODBUS_FC_READ_DISCRETE_INPUTS    :
            case MODBUS_FC_READ_HOLDING_REGISTERS  :
            case MODBUS_FC_READ_INPUT_REGISTERS    :
            case MODBUS_FC_WRITE_MULTIPLE_COILS    :
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                info.rangeSize = GET_HDR_U16(1);
                break;
            case MODBUS_FC_WRITE_SINGLE_COIL       :
            case MODBUS_FC_WRITE_SINGLE_REGISTER   :
            case MODBUS_FC_MASK_WRITE_REGISTER     :
            default:
                info.rangeSize = 1;
                break;
        }

        LMB_DGLOG(LDOM_HOOK, "try process hook on A = " << info.rangeBaseAddress
                  << "-" << info.rangeSize
                  << "  T = " << (int)info.type
                  << "  F = " << info.function);

        tryProcessUniHook(info);

#undef GET_HDR_U16

    }

    void beforeStartListen() {
        auto it = m_uniHook.begin();
        while(it != m_uniHook.end()) {
            it->compile();
            ++it;
        }
    }
};

}



AbstractSlaveBackend::AbstractSlaveBackend()
    : ad_ptr(new AbstractSlaveBackendPrivate(this))
{
    d_ptr = ad_ptr.data();
}

void AbstractSlaveBackend::processHooks(const uint8_t *req, int req_length, HookTime hookTime)
{
    LMB_DGLOG(LDOM_HOOK, "process event " << (hookTime == HookTime::Preprocessing ? "pre" : "post"));
    if (hookTime == HookTime::Preprocessing) {
        d_ptr->checkHookMap(req, req_length, d_ptr->m_hooks, hookTime);
    } else {
        d_ptr->checkHookMap(req, req_length, d_ptr->m_postMessageHooks, hookTime);
    }
}

AbstractSlaveBackend::~AbstractSlaveBackend()
{
    modbus_mapping_free(d_ptr->m_map);
}

modbus_mapping_t *AbstractSlaveBackend::getMap() const {
    return d_ptr->m_map;
}

bool AbstractSlaveBackend::initMap(int holdingBitsCount, int inputBitsCount, int holdingRegistersCount, int inputRegistersCount)
{
    d_ptr->m_map = modbus_mapping_new(holdingBitsCount, inputBitsCount, holdingRegistersCount, inputRegistersCount);
    return (d_ptr->m_map != Q_NULLPTR);
}

bool AbstractSlaveBackend::initRegisterMap(int holdingRegistersCount, int inputRegistersCount)
{
    return initMap(0, 0, holdingRegistersCount, inputRegistersCount);
}

bool AbstractSlaveBackend::startListen()
{
    d_ptr->beforeStartListen();
    return doStartListen();
}

void AbstractSlaveBackend::stopListen()
{
    doStopListen();
}

void AbstractSlaveBackend::addPreMessageHook(FunctionCode funcCode, Address address, HookFunction func)
{
    d_ptr->m_hooks[funcCode][address] = func;
}

void AbstractSlaveBackend::addPostMessageHook(FunctionCode funcCode, Address address, HookFunction func)
{
    d_ptr->m_postMessageHooks[funcCode][address] = func;
}

int AbstractSlaveBackend::customSelect(modbus_t *ctx, fd_set *rset, timeval *tv, int msg_length, QIODevice *dev)
{
    Q_UNUSED(ctx);
    Q_UNUSED(rset);
    Q_UNUSED(tv);

    const double timeous_ms =
            (tv)
            ? (tv->tv_sec * 1000.0 + tv->tv_usec / 1000.0)
            : 0.0;

    LMB_DGLOG(LDOM_IO,
             "select. Wait for " << msg_length << " bytes. " <<
             "Avail" << (dev ? dev->bytesAvailable() : -1) <<
             "TO = " << (tv ? timeous_ms : -1.0));

    Q_UNUSED(ctx);
    Q_UNUSED(rset);


    if (!dev) {
        LMB_WGLOG(LDOM_IO, "IO is nil");
        return -1;
    }

    QTime timeoutTimer;
    timeoutTimer.start();

    qint64 prevAvail = dev->bytesAvailable();
    while(dev->bytesAvailable() < msg_length) {
        if (tv) {
            if (timeoutTimer.elapsed() > timeous_ms)  {
                LMB_WGLOG(LDOM_IO, "Timeout! Elapsed[ms] = " << timeoutTimer.elapsed());
                return 0; // no descriptors signaled
            }
        }

        QEventLoop el;
        static const int pollInterval_ms = 50;
        el.processEvents(QEventLoop::AllEvents, pollInterval_ms);

        if (!dev) {
            LMB_WGLOG(LDOM_IO, "IO is nil");
            return -1;
        }

        const qint64 avail = dev->bytesAvailable();
        if (avail != prevAvail) {
            LMB_DGLOG(LDOM_IO, "avail = " << dev->bytesAvailable());
            prevAvail = avail;
        }
    }

    return 1; // count if signaled descriptors
}


ssize_t AbstractSlaveBackend::customRecv(modbus_t *ctx, uint8_t *rsp, int rsp_length, QIODevice *dev)
{
    Q_UNUSED(ctx);

    const int readedCount = dev->read(reinterpret_cast<char*>(rsp), rsp_length);

    LMB_DGLOG(LDOM_IO, "recv size = " << readedCount);
    if (readedCount > 0) {
        LMB_DGLOG(LDOM_IO, "recv data = " << BUF2HEX(rsp, readedCount));
    }

    return readedCount;
}


ssize_t AbstractSlaveBackend::customSend(modbus_t *ctx, const uint8_t *rsp, int rsp_length, QIODevice *dev)
{
    Q_UNUSED(ctx);

    LMB_DGLOG(LDOM_IO, "send size = " << rsp_length);
    if (rsp_length > 0) {
        LMB_DGLOG(LDOM_IO, "send data = " << BUF2HEX(rsp, rsp_length));
    }

    const int writedCount = dev->write(reinterpret_cast<const char*>(rsp), rsp_length);

    LMB_DGLOG(LDOM_IO, "sent = " << writedCount);

    return writedCount;
}


void AbstractSlaveBackend::addUniHook(DataType type, AccessMode accessMode, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func)
{
    const UniHookKey key = uniHookKey(type, accessMode, hookTime);
    d_ptr->m_uniHook[key].add(rangeBaseAddress, rangeSize, func);

    LMB_DGLOG(LDOM_HOOK, "add hook for " << key << ". Count = " << d_ptr->m_uniHook[key].count());
}
