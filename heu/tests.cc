//
// Created by HqZHao on 2023/07/10.
//
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  std::printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
