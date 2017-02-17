#include "tests/tcp_read_write_test.h"
#include <QThreadPool>
#include <libmodbus_cpp/master_tcp.h>
#include <thread>

void libmodbus_cpp::TcpReadWriteTest::initTestCase()
{
    m_serverStarter = new TcpServerStarter;
    //QObject::connect(this, &TcpReadWriteTest::sig_finished, m_serverStarter, &TcpServerStarter::slot_stop, Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sig_finished()), m_serverStarter, SLOT(slot_stop()));
    QThreadPool::globalInstance()->start(m_serverStarter);
    while (!m_serverStarter->isReady())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    m_master.reset(Factory::createTcpMaster(TEST_IP_ADDRESS, TEST_PORT).release());
}

void libmodbus_cpp::TcpReadWriteTest::cleanupTestCase()
{
    emit sig_finished();
    // m_serverStarter will be deleted by thread pool
}
