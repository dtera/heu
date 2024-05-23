//
// Created by HqZhao on 2024/05/21.
//

#include <gtest/gtest.h>

#include <iostream>

#include "stopwatch.hpp"

#include "heu/algorithms/seal_fhe/gpu/he_kit.cuh"

using namespace seal_gpun;
using namespace heu::algos::seal_fhe::gpu;

TEST(heu, seal_fhe_gpu) {
  size_t iter = 10000;
  StopWatch sw;
  std::cout << "----- CKKS -----\n";
  sw.Mark("HeKit::Create");
  auto hekit = HeKit::Create(SchemeType::ckks, 4096);
  sw.PrintWithMills("HeKit::Create");
  std::cout << std::endl;

  std::vector<int64_t> ms1(iter), ms2(iter), res(iter);
  std::vector<seal_gpun::Plaintext> pts1(iter), pts2(iter), pts(iter);
  std::vector<seal_gpun::Ciphertext> cts1(iter), cts2(iter), out(iter);
  for (int i = 0; i < iter; i++) {
    ms1[i] = i + 1;
    ms2[i] = i * 100 + 10;
  }

  sw.Mark("HeKit::Encode1");
  hekit->Encode(ms1, pts1);
  sw.PrintWithMills("HeKit::Encode1");
  sw.Mark("HeKit::Encode2");
  hekit->Encode(ms2, pts2);
  sw.PrintWithMills("HeKit::Encode2");
  std::cout << std::endl;

  sw.Mark("HeKit::Encrypt1");
  hekit->Encrypt(pts1, cts1, true);
  sw.PrintWithMills("HeKit::Encrypt1");
  sw.Mark("HeKit::Encrypt2");
  hekit->Encrypt(pts2, cts2, true);
  sw.PrintWithMills("HeKit::Encrypt2");
  std::cout << std::endl;

  sw.Mark("HeKit::Add");
  hekit->Add(cts1, cts2, out);
  sw.PrintWithMills("HeKit::Add");
  sw.Mark("HeKit::Sub");
  hekit->Sub(cts1, cts2, out);
  sw.PrintWithMills("HeKit::Sub");
  sw.Mark("HeKit::AddInplace");
  hekit->AddInplace(cts1, cts2);
  sw.PrintWithMills("HeKit::AddInplace");
  sw.Mark("HeKit::SubInplace");
  hekit->SubInplace(cts1, cts2);
  sw.PrintWithMills("HeKit::SubInplace");
  std::cout << std::endl;

  sw.Mark("HeKit::Decrypt");
  hekit->Decrypt(cts1, pts);
  sw.PrintWithMills("HeKit::Decrypt");
  std::cout << std::endl;
}
