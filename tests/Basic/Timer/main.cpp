#include "gtest/gtest.h"
#include "System/Timer.h"
#include "System/Dispatcher.h"

using namespace System;

// constructor
TEST(Timer, 1)
{
  Timer();
}

// constructor
TEST(Timer, 2)
{
  Dispatcher dispatcher;
  Timer timer(dispatcher);
}

// constructor
TEST(Timer, 3)
{
  Timer timer1;
  Timer timer2(std::move(timer1));
}

// constructor
TEST(Timer, 4)
{
  Timer timer1;
  Timer timer2 = std::move(timer1);
}

// sleep()
TEST(Timer, 5)
{
  Dispatcher dispatcher;
  Timer timer(dispatcher);
  std::chrono::nanoseconds duration = std::chrono::nanoseconds{1000};
  timer.sleep(duration);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}