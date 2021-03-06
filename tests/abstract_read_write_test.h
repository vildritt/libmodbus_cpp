#ifndef LIBMODBUS_CPP_REMOTEREADWRITETEST_H
#define LIBMODBUS_CPP_REMOTEREADWRITETEST_H

#include <QObject>
#include <QtTest/QtTest>

#include <libmodbus_cpp/abstract_master.h>

namespace libmodbus_cpp {

const int TABLE_SIZE = 64;

class AbstractReadWriteTest : public QObject
{
    Q_OBJECT

protected:
    QScopedPointer<libmodbus_cpp::AbstractMaster> m_master;

private slots:
    virtual void initTestCase() = 0;
    void testConnection();
    void readCoils();
    void readVectorOfCoils();
    void writeCoils();
    void writeVectorOfCoils();
    void readDiscreteInputs();
    void readVectorOfDiscreteInputs();
    void readInputRegisters_int8();
    void readInputRegisters_uint8();
    void readInputRegisters_int16();
    void readInputRegisters_uint16();
    void readInputRegisters_int32();
    void readInputRegisters_uint32();
    void readInputRegisters_float();
    void readInputRegisters_int64();
    void readInputRegisters_uint64();
    void readInputRegisters_double();
    void readHoldingRegisters_int8();
    void readHoldingRegisters_uint8();
    void readHoldingRegisters_int16();
    void readHoldingRegisters_uint16();
    void readHoldingRegisters_int32();
    void readHoldingRegisters_uint32();
    void readHoldingRegisters_float();
    void readHoldingRegisters_int64();
    void readHoldingRegisters_uint64();
    void readHoldingRegisters_double();
    void writeReadHoldingRegisters_int8();
    void writeReadHoldingRegisters_uint8();
    void writeReadHoldingRegisters_int16();
    void writeReadHoldingRegisters_uint16();
    void writeReadHoldingRegisters_int32();
    void writeReadHoldingRegisters_uint32();
    void writeReadHoldingRegisters_float();
    void writeReadHoldingRegisters_int64();
    void writeReadHoldingRegisters_uint64();
    void writeReadHoldingRegisters_double();
    virtual void cleanupTestCase() = 0;

private:
    void connect();
    void disconnect();

    template<typename ValueType>
    void testReadFromInputRegisters() {
        connect();
        int size = std::max(sizeof(ValueType) / sizeof(uint16_t), static_cast<size_t>(1u));
        for (int i = 0; i < TABLE_SIZE; i += size) {
            ValueType valueBefore = 0;
            for (int j = 0; j < size; ++j)
                reinterpret_cast<uint16_t*>(&valueBefore)[j] = 1;
            try {
                ValueType valueAfter = m_master->readInputRegister<ValueType>(i);
                QCOMPARE(valueAfter, valueBefore);
            } catch (RemoteRWError &e) {
                QVERIFY2(false, e.what());
            }
        }
        disconnect();
    }

    template<typename ValueType>
    void testReadFromHoldingRegisters() {
        connect();
        int size = std::max(sizeof(ValueType) / sizeof(uint16_t), static_cast<size_t>(1u));
        for (int i = 0; i < TABLE_SIZE; i += size) {
            ValueType valueBefore = 0;
            for (int i = 0; i < size; ++i)
                reinterpret_cast<uint16_t*>(&valueBefore)[i] = 1;
            try {
                ValueType valueAfter = m_master->readHoldingRegister<ValueType>(i);
                QCOMPARE(valueAfter, valueBefore);
            } catch (RemoteRWError &e) {
                QVERIFY2(false, e.what());
            }
        }
        disconnect();
    }


    template<typename ValueType>
    void testWriteToHoldingRegisters() {
        connect();
        int size = std::max(sizeof(ValueType) / sizeof(uint16_t), static_cast<size_t>(1u));
        for (int i = 0; i < TABLE_SIZE; i += size) {
            ValueType valueBefore = (ValueType)(rand()) + 1 / (double)rand();
            try {
                m_master->writeHoldingRegister(i, valueBefore);
                ValueType valueAfter = m_master->readHoldingRegister<ValueType>(i);
                QCOMPARE(valueAfter, valueBefore);
            } catch (RemoteRWError &e) {
                QVERIFY2(false, e.what());
            }
        }
        disconnect();
    }
};

}

#endif // LIBMODBUS_CPP_REMOTEREADWRITETEST_H
