
#include "stopwatch.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

TEST(stopwatch, show) {
  StopWatch sw;
  for (int i = 0; i < 10000; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::cout << "ShowTickNano: " << sw.ShowTickNano() << std::endl;
  std::cout << "ShowTickMicro: " << sw.ShowTickMicro() << std::endl;
  std::cout << "ShowTickMills: " << sw.ShowTickMills() << std::endl;
  std::cout << "ShowTickSeconds: " << sw.ShowTickSeconds() << std::endl;
  std::cout << "ShowTickMinutes: " << sw.ShowTickMinutes() << std::endl;
  std::cout << "ShowTickHours: " << sw.ShowTickHours() << std::endl;
}
