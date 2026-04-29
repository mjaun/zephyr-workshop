#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cpp_wrappers/Buzzer.hpp"
#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "BuzzerController.hpp"

K_THREAD_STACK_DEFINE(thread_stack, 1024);

class BuzzerMock : public IBuzzer
{
public:
    MOCK_METHOD(void, enable, (uint32_t freq));
    MOCK_METHOD(void, disable, ());
};

class BuzzerControllerTest : public testing::Test
{
protected:
    BuzzerMock _buzzer_mock;

    Thread _thread{thread_stack, K_THREAD_STACK_SIZEOF(thread_stack), 5};
    MessageQueue<float> _message_queue{3};
    BuzzerController _buzzer_controller{_thread, _message_queue, _buzzer_mock};
};

TEST_F(BuzzerControllerTest, AboveThresholdDisablesBuzzer)
{
    _message_queue.put(8.1f);

    EXPECT_CALL(_buzzer_mock, disable());

    _buzzer_controller.start();
    k_msleep(1);
}

TEST_F(BuzzerControllerTest, AtMinAccelEnablesWithMinFreq)
{
    _message_queue.put(8.0f);

    EXPECT_CALL(_buzzer_mock, enable(500));

    _buzzer_controller.start();
    k_msleep(1);
}

TEST_F(BuzzerControllerTest, AtMaxAccelEnablesWithMaxFreq)
{
    _message_queue.put(-9.0f);

    EXPECT_CALL(_buzzer_mock, enable(3000));

    _buzzer_controller.start();
    k_msleep(1);
}

TEST_F(BuzzerControllerTest, AtMidPointEnablesWithFreq)
{
    _message_queue.put(-0.5f);

    /* accel = -0.5: 500 + (-0.5 - 8) * 2500 / -17 = 1750 */
    EXPECT_CALL(_buzzer_mock, enable(1750));

    _buzzer_controller.start();
    k_msleep(1);
}
