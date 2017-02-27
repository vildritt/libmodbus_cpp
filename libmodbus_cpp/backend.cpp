#include <cassert>
#include <modbus/modbus-private.h>
#include <libmodbus_cpp/backend.h>
#include <QVector>

using namespace libmodbus_cpp;

AbstractBackend::AbstractBackend()
{
}

bool AbstractBackend::doesSystemByteOrderMatchTarget() const
{
    return targetByteOrder == systemByteOrder;
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

ByteOrder AbstractBackend::checkSystemByteOrder()
{
    union {
        unsigned short s;
        unsigned char c[2];
    } x { 0x0201 };
    return (x.c[1] > x.c[0]) ? ByteOrder::LittleEndian : ByteOrder::BigEndian;
}


namespace  libmodbus_cpp {

class AbstractSlaveBackendPrivate {
public:
    using HooksByAddress = QMap<Address, HookFunction>;
    using HooksByFunctionCode = QMap<FunctionCode, HooksByAddress>;
    using UniHookKey = int;

    struct UniHookSetup {
        Address rangeBaseAddress;
        Address rangeLength;
        UniHookFunction handler;
    };

    struct UniHooks {
        QVector<UniHookSetup> hooks;

        void add(Address rangeBaseAddress, Address rangeLength, UniHookFunction func) {
            UniHookSetup stp;
            stp.handler = func;
            stp.rangeBaseAddress = rangeBaseAddress;
            stp.rangeLength = rangeLength;
            hooks.append(stp);
        }

        void compile() {
            qSort(hooks.begin(), hooks.end(), [](const UniHookSetup &L, const UniHookSetup &R) -> bool{
                if (L.rangeBaseAddress == R.rangeBaseAddress) {
                    return L.rangeLength < R.rangeLength;
                } else {
                    return L.rangeBaseAddress < R.rangeBaseAddress;
                }
            });
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


    void checkHookMap(const uint8_t *req, int req_length, const HooksByFunctionCode &hooks) {
        Q_UNUSED(req_length);
        const int offset = modbus_get_header_length(q->getCtx());
        const FunctionCode function = req[offset];
        const Address address = (req[offset + 1] << 8) + req[offset + 2];

        const auto &hooksa = hooks[function];
        if (hooksa.contains(address)) {
            hooksa[address]();
        }
    }

    void beforeStartListen() {
        auto it = m_uniHook.begin();
        while(it == m_uniHook.end()) {
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
    if (hookTime == HookTime::Preprocessing) {
        d_ptr->checkHookMap(req, req_length, d_ptr->m_hooks);
    } else {
        d_ptr->checkHookMap(req, req_length, d_ptr->m_postMessageHooks);
    }
}

AbstractSlaveBackend::~AbstractSlaveBackend()
{
    modbus_mapping_free(d_ptr->m_map);
}

modbus_mapping_t *AbstractSlaveBackend::getMap() {
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

static AbstractSlaveBackendPrivate::UniHookKey uniHookKey(const AccessMode accessMode, const HookTime hookTime)
{
    return
            (static_cast<int>(accessMode) << 8) +
            (static_cast<int>(hookTime));
}

void AbstractSlaveBackend::addUniHook(AccessMode accessMode, Address rangeBaseAddress, Address rangeSize, HookTime hookTime, UniHookFunction func)
{
    d_ptr->m_uniHook[uniHookKey(accessMode, hookTime)].add(rangeBaseAddress, rangeSize, func);
}
