// Copyright 2022 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef USE_CMAKE
#include <omp.h>

#include "utils.h"
#endif

#include <functional>
#include <future>
#include <mutex>

#include "benchmark/benchmark.h"
#include "fmt/ranges.h"
#include "gflags/gflags.h"

#include "heu/spi/he/he.h"

namespace heu::lib::bench {

constexpr static long kTestSize = 10000;

class FheBenchmarks {
 public:
  void SetupAndRegister(spi::Schema schema, int poly_modulus_degree) {
    he_kit_ = spi::HeFactory::Instance().Create(
        schema, spi::ArgPolyModulusDegree = poly_modulus_degree);
    auto edr = he_kit_->GetEncoder(spi::ArgScale = 1);
    for (uint64_t i = 0; i < kTestSize; ++i) {
      pts_[i] = edr->Encode(i);
    }

    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|Encrypt", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { Encrypt(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|CT+CT", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { AddCipher(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|CT-CT", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { SubCipher(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|CT+PT", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { AddInt(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|CT-PT", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { SubInt(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|CT*PT", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { Multi(st); })
        ->Unit(benchmark::kMillisecond);
    benchmark::RegisterBenchmark(
        fmt::format("{:^9}|Decrypt", he_kit_->GetSchema()).c_str(),
        [this](benchmark::State &st) { Decrypt(st); })
        ->Unit(benchmark::kMillisecond);
  }

  void Encrypt(benchmark::State &state) {
    std::call_once(flag_, []() { fmt::print("{:-^62}\n", ""); });

    // encrypt
    const auto &encryptor = he_kit_->GetEncryptor();
    for (auto _ : state) {
#ifdef USE_CMAKE
      if (parallel) {
        ParallelFor(kTestSize, n_thread,
                    [&](int i) { *(cts_ + i) = encryptor->Encrypt(pts_[i]); });
      } else {
#endif
        for (int i = 0; i < kTestSize; ++i) {
          *(cts_ + i) = encryptor->Encrypt(pts_[i]);
        }
#ifdef USE_CMAKE
      }
#endif
    }
  }

  void AddCipher(benchmark::State &state) {
    // add (ciphertext + ciphertext)
    const auto &evaluator = he_kit_->GetWordEvaluator();
    auto ct = he_kit_->GetEncryptor()->EncryptZero();
    for (auto _ : state) {
      for (int i = 0; i < kTestSize; ++i) {
        evaluator->AddInplace(&ct, cts_[i]);
      }
    }
  }

  void SubCipher(benchmark::State &state) {
    // sub (ciphertext - ciphertext)
    const auto &evaluator = he_kit_->GetWordEvaluator();
    auto ct = he_kit_->GetEncryptor()->EncryptZero();
    for (auto _ : state) {
      for (int i = 0; i < kTestSize; ++i) {
        evaluator->SubInplace(&ct, cts_[i]);
      }
    }
  }

  void AddInt(benchmark::State &state) {
    // add (ciphertext + plaintext)
    const auto &evaluator = he_kit_->GetWordEvaluator();
    auto edr = he_kit_->GetEncoder(spi::ArgScale = 1);
    for (auto _ : state) {
      for (uint64_t i = 0; i < kTestSize; ++i) {
        evaluator->AddInplace(&cts_[i], edr->Encode(i));
      }
    }
  }

  void SubInt(benchmark::State &state) {
    // add (ciphertext - plaintext)
    const auto &evaluator = he_kit_->GetWordEvaluator();
    auto edr = he_kit_->GetEncoder(spi::ArgScale = 1);
    for (auto _ : state) {
      for (uint64_t i = 0; i < kTestSize; ++i) {
        evaluator->SubInplace(&cts_[i], edr->Encode(i));
      }
    }
  }

  void Multi(benchmark::State &state) {
    // mul (ciphertext * plaintext)
    const auto &evaluator = he_kit_->GetWordEvaluator();
    auto edr = he_kit_->GetEncoder(spi::ArgScale = 1);
    auto ct = he_kit_->GetEncryptor()->Encrypt(edr->Encode(1L));
    for (auto _ : state) {
      for (uint64_t i = 1; i < kTestSize; ++i) {
        evaluator->MulInplace(&ct, edr->Encode(i));
      }
    }
  }

  void Decrypt(benchmark::State &state) {
    // decrypt
    const auto &decryptor = he_kit_->GetDecryptor();
    for (auto _ : state) {
#ifdef USE_CMAKE
      if (parallel) {
        ParallelFor(kTestSize, n_thread,
                    [&](int i) { decryptor->Decrypt(cts_[i], pts_ + i); });
      } else {
#endif
        for (int i = 0; i < kTestSize; ++i) {
          decryptor->Decrypt(cts_[i], pts_ + i);
        }
#ifdef USE_CMAKE
      }
#endif
    }
  }

 private:
  std::once_flag flag_;
  std::unique_ptr<spi::HeKit> he_kit_;
  spi::Item pts_[kTestSize];
  spi::Item cts_[kTestSize];
#ifdef USE_CMAKE
  bool parallel = false;
  int n_thread = 10;
#endif
};

}  // namespace heu::lib::bench

DEFINE_string(schema, "gpu_ckks", "Run selected schemas, default to gpu_ckks.");
DEFINE_int32(poly_modulus_degree, 2048, "Key size of fhe schema.");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  benchmark::Initialize(&argc, argv);
  benchmark::AddCustomContext("Run times",
                              fmt::format("{}", heu::lib::bench::kTestSize));
  benchmark::AddCustomContext("poly_modulus_degree",
                              fmt::format("{}", FLAGS_poly_modulus_degree));

  auto schema = heu::spi::String2Schema(FLAGS_schema);
  fmt::print("Schemas to bench: {}\n", schema);
  // heu::lib::bench::FheBenchmarks bm;
  // bm.SetupAndRegister(schema, FLAGS_poly_modulus_degree);

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
