#ifndef LIBMODBUSCPP_LOGGER_H_GUARD
#define LIBMODBUSCPP_LOGGER_H_GUARD

#include <QByteArray>
#include <QDebug>
#include "global.h"

#define LMB_LOG_LOCAL_CHECKER  m_verbose
#define LMB_LOG_GLOBAL_CHECKER libmodbus_cpp::isVerbose()

#define BUF2HEX(Buffer, Size) QByteArray(reinterpret_cast<const char*>(Buffer), Size).toHex().data()
#define LMB_LOG(Logger, Checker, Domain, Msg) \
    do { \
        if (Checker) { \
            Logger << Domain << Msg; \
        } \
    } while (0)


#define LMB_WLOG(Domain, Msg)   LMB_LOG(qWarning(), true,                  Domain, Msg)
#define LMB_ILOG(Domain, Msg)   LMB_LOG(qInfo(),    LMB_LOG_LOCAL_CHECKER, Domain, Msg)
#define LMB_DLOG(Domain, Msg)   LMB_LOG(qDebug(),   LMB_LOG_LOCAL_CHECKER, Domain, Msg)

#define LMB_WGLOG(Domain, Msg)  LMB_LOG(qWarning(), true,                   Domain, Msg)
#define LMB_IGLOG(Domain, Msg)  LMB_LOG(qInfo(),    LMB_LOG_GLOBAL_CHECKER, Domain, Msg)
#define LMB_DGLOG(Domain, Msg)  LMB_LOG(qDebug(),   LMB_LOG_GLOBAL_CHECKER, Domain, Msg)

#endif // LIBMODBUSCPP_LOGGER_H_GUARD

