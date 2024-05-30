//
// Created by HqZhao on 2024/05/21.
//

#include <gtest/gtest.h>

#include <iostream>

#include "stopwatch.hpp"
#include "utils.h"

#include "heu/algorithms/seal_fhe/gpu/he_kit.cuh"

using namespace seal_gpun;
using namespace heu::algos::seal_fhe::gpu;

TEST(seal_fhe_gpu, test1) {
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

#define M double

TEST(seal_fhe_gpu, test2) {
  StopWatch sw;
  std::cout << "----- CKKS -----\n";
  sw.Mark("HeKit::Create");
  auto hekit = HeKit::Create(SchemeType::ckks, 4096, 1 << 16);
  sw.PrintWithMills("HeKit::Create");
  std::cout << std::endl;

  size_t iter = hekit->SlotCount();
  std::vector<M> ms1(iter, 0), ms2(iter, 0), ms(iter, 0);
  seal_gpun::Plaintext pt1, pt2, pt;
  seal_gpun::Ciphertext ct1, ct2, out;
  M real = 0, res = 0;
  for (int i = 0; i < iter; i++) {
    ms1[i] = i + 1;
    ms2[i] = i + 2;
    real += ms1[i] * ms2[i];
  }
  std::cout << "待编码的向量ms1:";
  print_vector(ms1);
  std::cout << "待编码的向量ms2:";
  print_vector(ms2);

  sw.Mark("HeKit::Encode:pt1");
  hekit->Encode(ms1, pt1);
  sw.PrintWithMills("HeKit::Encode:pt1");
  sw.Mark("HeKit::Encode:pt2");
  hekit->Encode(ms2, pt2);
  sw.PrintWithMills("HeKit::Encode:pt2");
  std::cout << std::endl;

  sw.Mark("HeKit::Encrypt:ct1");
  hekit->Encrypt(pt1, ct1);
  sw.PrintWithMills("HeKit::Encrypt:ct1");
  sw.Mark("HeKit::Encrypt:ct2");
  hekit->Encrypt(pt2, ct2);
  sw.PrintWithMills("HeKit::Encrypt:ct2");
  std::cout << std::endl;

  hekit->Decrypt(ct2, pt);
  hekit->Decode(pt, ms);
  std::cout << "解密ms2向量res:";
  print_vector(ms);

  sw.Mark("HeKit::Multiply");
  hekit->Multiply(ct1, ct2, out);
  sw.PrintWithMills("HeKit::Multiply");
  /*sw.Mark("HeKit::RotateSum");
  hekit->RotateSum(out);
  sw.PrintWithMills("HeKit::RotateSum");*/
  std::cout << std::endl;

  sw.Mark("HeKit::Decrypt");
  hekit->Decrypt(out, pt);
  sw.PrintWithMills("HeKit::Decrypt");
  std::cout << std::endl;

  sw.Mark("HeKit::Decode");
  // res = hekit->Decode(pt);
  hekit->Decode(pt, ms);
  sw.PrintWithMills("HeKit::Decode");
  // std::cout << "real: " << real << ", res:" << res << std::endl;
  for (int i = 0; i < iter; i++) {
    if (i % 256 == 0) {
      std::cout << "i: " << i << "\t real: " << ms1[i] * ms2[i]
                << "\t\t res:" << ms[i] << std::endl;
    }
  }
}
