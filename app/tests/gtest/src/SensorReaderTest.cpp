#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cpp_wrappers/Accelerometer.hpp"
#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "SensorReader.hpp"

using testing::Return;

K_THREAD_STACK_DEFINE(sensor_reader_thread_stack, 1024);

class AccelerometerMock : public IAccelerometer
{
public:
    MOCK_METHOD(float, read, ());
};

class SensorReaderTest : public testing::Test
{
protected:
    AccelerometerMock _accelerometer_mock;

    Thread _thread{sensor_reader_thread_stack, K_THREAD_STACK_SIZEOF(sensor_reader_thread_stack), 5};
    MessageQueue<float> _message_queue{3};
    SensorReader _sensor_reader{_thread, _message_queue, _accelerometer_mock};
};

TEST_F(SensorReaderTest, NoReadBeforeFirstPeriod)
{
    EXPECT_CALL(_accelerometer_mock, read()).Times(0);

    _sensor_reader.start();
    k_msleep(10);
}

TEST_F(SensorReaderTest, ReadValueIsPutIntoQueue)
{
    EXPECT_CALL(_accelerometer_mock, read()).WillOnce(Return(1.5f));

    _sensor_reader.start();
    k_msleep(30);

    EXPECT_FLOAT_EQ(_message_queue.get(), 1.5f);
}

TEST_F(SensorReaderTest, ReadsPeriodicallyInOrder)
{
    EXPECT_CALL(_accelerometer_mock, read())
        .WillOnce(Return(-9.0f))
        .WillOnce(Return(0.0f))
        .WillOnce(Return(8.0f));

    _sensor_reader.start();
    k_msleep(70);

    EXPECT_FLOAT_EQ(_message_queue.get(), -9.0f);
    EXPECT_FLOAT_EQ(_message_queue.get(), 0.0f);
    EXPECT_FLOAT_EQ(_message_queue.get(), 8.0f);
}
