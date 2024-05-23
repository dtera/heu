// Copyright 2024 dterazhao Co., Ltd.
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

#pragma once
#include <string>

#include "utils.h"

#include "heu/library/algorithms/seal_gpu/seal_cuda.cuh"

namespace heu::algos::seal_fhe::gpu {

class HeKit {
 public:
  static std::shared_ptr<HeKit> Create(
      const seal_gpun::EncryptionParameters &params);

  static std::shared_ptr<HeKit> Create(const seal_gpun::SchemeType &scheme,
                                       const uint64_t poly_modulus_degree,
                                       const std::vector<int> &bit_sizes,
                                       const double scale = 1 << 6);

  static std::shared_ptr<HeKit> Create(const seal_gpun::SchemeType &scheme,
                                       const uint64_t poly_modulus_degree,
                                       const double scale = 1 << 6);

  HeKit(const seal_gpun::EncryptionParameters &params);
  HeKit(const seal_gpun::SchemeType &scheme, const uint64_t poly_modulus_degree,
        const std::vector<int> &bit_sizes, const double scale = 1 << 6);
  HeKit(const seal_gpun::SchemeType &scheme, const uint64_t poly_modulus_degree,
        const double scale = 1 << 6);
  ~HeKit();

  std::string GetLibraryName() const;

  inline const seal_gpun::Encryptor *GetEncryptor() const { return encryptor_; }

  inline const seal_gpun::Decryptor *GetDecryptor() const { return decryptor_; }

  inline const seal_gpun::Evaluator *GetEvaluator() const { return evaluator_; }

  inline const seal_gpun::CKKSEncoder *GetCKKSEncoder() const {
    return ckks_encoder_;
  }

  inline const seal_gpun::BatchEncoder *GetBatchEncoder() const {
    return batch_encoder_;
  }

  //==========================fhe_gpu operation bigin==========================
  void Encode(const int64_t m, seal_gpun::Plaintext &out);
  void Encode(const double m, seal_gpun::Plaintext &out);
  template <typename T,
            typename std::enable_if_t<std::is_arithmetic_v<T>, int64_t> = 0>
  void Encode(const std::vector<T> &ms, std::vector<seal_gpun::Plaintext> &out,
              bool async = false, int32_t n_threads = omp_get_num_procs());

  void Encrypt(const seal_gpun::Plaintext &pt, seal_gpun::Ciphertext &out);
  template <typename T,
            typename std::enable_if_t<std::is_arithmetic_v<T>, int64_t> = 0>
  void Encrypt(const T m, seal_gpun::Ciphertext &out);
  template <typename T>
  void Encrypt(const std::vector<T> &pts,
               std::vector<seal_gpun::Ciphertext> &out, bool async = false,
               int32_t n_threads = omp_get_num_procs());

  void Decrypt(const seal_gpun::Ciphertext &ct, seal_gpun::Plaintext &out);
  void Decrypt(const std::vector<seal_gpun::Ciphertext> &cts,
               std::vector<seal_gpun::Plaintext> &out, bool async = false,
               int32_t n_threads = omp_get_num_procs());

  void Eval(const std::vector<seal_gpun::Ciphertext> &cts1,
            const std::vector<seal_gpun::Ciphertext> &cts2,
            std::vector<seal_gpun::Ciphertext> &out,
            std::function<void(const seal_gpun::Ciphertext &,
                               const seal_gpun::Ciphertext &,
                               seal_gpun::Ciphertext &)>
                eval_func,
            bool async = false, int32_t n_threads = omp_get_num_procs());
  void EvalInplace(std::vector<seal_gpun::Ciphertext> &cts1,
                   const std::vector<seal_gpun::Ciphertext> &cts2,
                   std::function<void(seal_gpun::Ciphertext &,
                                      const seal_gpun::Ciphertext &)>
                       eval_func,
                   bool async = false, int32_t n_threads = omp_get_num_procs());

  void Add(const seal_gpun::Ciphertext &ct1, const seal_gpun::Ciphertext &ct2,
           seal_gpun::Ciphertext &out);
  void Add(const std::vector<seal_gpun::Ciphertext> &cts1,
           const std::vector<seal_gpun::Ciphertext> &cts2,
           std::vector<seal_gpun::Ciphertext> &out, bool async = false,
           int32_t n_threads = omp_get_num_procs());

  void AddInplace(seal_gpun::Ciphertext &ct1, const seal_gpun::Ciphertext &ct2);
  void AddInplace(std::vector<seal_gpun::Ciphertext> &cts1,
                  const std::vector<seal_gpun::Ciphertext> &cts2,
                  bool async = false, int32_t n_threads = omp_get_num_procs());

  void Sub(const seal_gpun::Ciphertext &ct1, const seal_gpun::Ciphertext &ct2,
           seal_gpun::Ciphertext &out);
  void Sub(const std::vector<seal_gpun::Ciphertext> &cts1,
           const std::vector<seal_gpun::Ciphertext> &cts2,
           std::vector<seal_gpun::Ciphertext> &out, bool async = false,
           int32_t n_threads = omp_get_num_procs());

  void SubInplace(seal_gpun::Ciphertext &ct1, const seal_gpun::Ciphertext &ct2);
  void SubInplace(std::vector<seal_gpun::Ciphertext> &cts1,
                  const std::vector<seal_gpun::Ciphertext> &cts2,
                  bool async = false, int32_t n_threads = omp_get_num_procs());
  //==========================fhe_gpu operation end============================

 private:
  void Init(const seal_gpun::EncryptionParameters &params);

  double scale_;
  double slot_count_;

  seal_gpun::PublicKey pk_;
  seal_gpun::RelinKeys rlk_;
  seal_gpun::GaloisKeys glk_;

  seal_gpu::KeyGeneratorCuda *keygen_;
  seal_gpun::SEALContext *context_;
  seal_gpun::Encryptor *encryptor_;
  seal_gpun::Decryptor *decryptor_;
  seal_gpun::Evaluator *evaluator_;
  seal_gpun::CKKSEncoder *ckks_encoder_;
  seal_gpun::BatchEncoder *batch_encoder_ = nullptr;
};

}  // namespace heu::algos::seal_fhe::gpu
