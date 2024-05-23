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

#include "heu/algorithms/seal_fhe/gpu/he_kit.cuh"

namespace heu::algos::seal_fhe::gpu {

static const std::string kLibName = "seal_fhe_gpu";  // do not change
static const std::map<int, std::vector<int>> poly_degree_bitsizes = {
    {1024, {20, 20}},
    {2048, {25, 25}},
    {4096, {60, 49}},
    {8192, {60, 49, 49}},
    {16384, {60, 40, 40, 40, 40, 60}},
    {32768, {60, 40, 40, 40, 40, 60}},
};  // do not change

std::shared_ptr<HeKit> HeKit::Create(
    const seal_gpun::EncryptionParameters &params) {
  return std::make_shared<HeKit>(params);
}

std::shared_ptr<HeKit> HeKit::Create(const seal_gpun::SchemeType &scheme,
                                     const uint64_t poly_modulus_degree,
                                     const std::vector<int> &bit_sizes,
                                     const double scale) {
  return std::make_shared<HeKit>(scheme, poly_modulus_degree, bit_sizes, scale);
}

std::shared_ptr<HeKit> HeKit::Create(const seal_gpun::SchemeType &scheme,
                                     const uint64_t poly_modulus_degree,
                                     const double scale) {
  return std::make_shared<HeKit>(scheme, poly_modulus_degree, scale);
}

HeKit::HeKit(const seal_gpun::EncryptionParameters &params) : scale_(1 << 6) {
  Init(params);
}

HeKit::HeKit(const seal_gpun::SchemeType &scheme,
             const uint64_t poly_modulus_degree,
             const std::vector<int> &bit_sizes, const double scale)
    : scale_(scale) {
  seal_gpun::KernelProvider::initialize();

  seal_gpu::EncryptionParameters params(scheme);
  params.setPolyModulusDegree(poly_modulus_degree);
  params.setCoeffModulus(
      seal_gpu::CoeffModulus::Create(poly_modulus_degree, bit_sizes));

  Init(params);
}

HeKit::HeKit(const seal_gpun::SchemeType &scheme,
             const uint64_t poly_modulus_degree, const double scale)
    : scale_(scale) {
  seal_gpun::KernelProvider::initialize();

  seal_gpu::EncryptionParameters params(scheme);
  params.setPolyModulusDegree(poly_modulus_degree);
  params.setCoeffModulus(seal_gpu::CoeffModulus::Create(
      poly_modulus_degree, poly_degree_bitsizes.at(poly_modulus_degree)));

  Init(params);
}

HeKit::~HeKit() {
  if (keygen_) delete keygen_;
  if (context_) delete context_;
  if (encryptor_) delete encryptor_;
  if (decryptor_) delete decryptor_;
  if (evaluator_) delete evaluator_;
  if (ckks_encoder_) delete ckks_encoder_;
  if (batch_encoder_) delete batch_encoder_;
}

std::string HeKit::GetLibraryName() const { return kLibName; }

void HeKit::Init(const seal_gpun::EncryptionParameters &params) {
  slot_count_ = params.polyModulusDegree() / 2;
  context_ = new seal_gpun::SEALContext(params);
  keygen_ = new seal_gpun::KeyGenerator(*context_);
  pk_ = keygen_->createPublicKey();
  rlk_ = keygen_->createRelinKeys();
  glk_ = keygen_->createGaloisKeys();

  encryptor_ = new seal_gpun::Encryptor(*context_, pk_);
  decryptor_ = new seal_gpun::Decryptor(*context_, keygen_->secretKey());
  evaluator_ = new seal_gpun::Evaluator(*context_);
  ckks_encoder_ = new seal_gpun::CKKSEncoder(*context_);
  if (params.scheme() == seal_gpun::SchemeType::bfv ||
      params.scheme() == seal_gpun::SchemeType::bgv) {
    batch_encoder_ = new seal_gpun::BatchEncoder(*context_);
  }
}

//==========================fhe_gpu operation bigin==========================
void HeKit::Encode(const int64_t m, seal_gpun::Plaintext &out) {
  ckks_encoder_->encode(m, out);
}

void HeKit::Encode(const double m, seal_gpun::Plaintext &out) {
  ckks_encoder_->encode(m, scale_, out);
}

template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int64_t>>
void HeKit::Encode(const std::vector<T> &ms,
                   std::vector<seal_gpun::Plaintext> &out, bool async,
                   int32_t n_threads) {
  if (async) {
    ParallelFor(ms.size(), n_threads, [&](int i) { Encode(ms[i], out[i]); });
  } else {
    for (int i = 0; i < ms.size(); i++) {
      Encode(ms[i], out[i]);
    }
  }
}

void HeKit::Encrypt(const seal_gpun::Plaintext &pt,
                    seal_gpun::Ciphertext &out) {
  encryptor_->encrypt(pt, out);
}

template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int64_t>>
void HeKit::Encrypt(const T m, seal_gpun::Ciphertext &out) {
  seal_gpun::Plaintext pt;
  Encode(m, pt);
  encryptor_->encrypt(pt, out);
}

template <typename T>
void HeKit::Encrypt(const std::vector<T> &pts,
                    std::vector<seal_gpun::Ciphertext> &out, bool async,
                    int32_t n_threads) {
  if (async) {
    ParallelFor(pts.size(), n_threads, [&](int i) { Encrypt(pts[i], out[i]); });
  } else {
    for (int i = 0; i < pts.size(); i++) {
      Encrypt(pts[i], out[i]);
    }
  }
}

void HeKit::Decrypt(const seal_gpun::Ciphertext &ct,
                    seal_gpun::Plaintext &out) {
  decryptor_->decrypt(ct, out);
}

void HeKit::Decrypt(const std::vector<seal_gpun::Ciphertext> &cts,
                    std::vector<seal_gpun::Plaintext> &out, bool async,
                    int32_t n_threads) {
  if (async) {
    ParallelFor(cts.size(), n_threads, [&](int i) { Decrypt(cts[i], out[i]); });
  } else {
    for (int i = 0; i < cts.size(); i++) {
      Decrypt(cts[i], out[i]);
    }
  }
}

void HeKit::Eval(
    const std::vector<seal_gpun::Ciphertext> &cts1,
    const std::vector<seal_gpun::Ciphertext> &cts2,
    std::vector<seal_gpun::Ciphertext> &out,
    std::function<void(const seal_gpun::Ciphertext &,
                       const seal_gpun::Ciphertext &, seal_gpun::Ciphertext &)>
        eval_func,
    bool async, int32_t n_threads) {
  auto len = cts1.size();
  if (async) {
    ParallelFor(len, n_threads,
                [&](int i) { eval_func(cts1[i], cts2[i], out[i]); });
  } else {
    for (int i = 0; i < len; i++) {
      eval_func(cts1[i], cts2[i], out[i]);
    }
  }
}

void HeKit::EvalInplace(
    std::vector<seal_gpun::Ciphertext> &cts1,
    const std::vector<seal_gpun::Ciphertext> &cts2,
    std::function<void(seal_gpun::Ciphertext &, const seal_gpun::Ciphertext &)>
        eval_func,
    bool async, int32_t n_threads) {
  auto len = cts1.size();
  if (async) {
    ParallelFor(len, n_threads, [&](int i) { eval_func(cts1[i], cts2[i]); });
  } else {
    for (int i = 0; i < len; i++) {
      eval_func(cts1[i], cts2[i]);
    }
  }
}

void HeKit::Add(const seal_gpun::Ciphertext &ct1,
                const seal_gpun::Ciphertext &ct2, seal_gpun::Ciphertext &out) {
  evaluator_->add(ct1, ct2, out);
}

void HeKit::Add(const std::vector<seal_gpun::Ciphertext> &cts1,
                const std::vector<seal_gpun::Ciphertext> &cts2,
                std::vector<seal_gpun::Ciphertext> &out, bool async,
                int32_t n_threads) {
  auto eval_func =
      std::bind(&seal_gpun::Evaluator::add, evaluator_, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  Eval(cts1, cts2, out, eval_func, async, n_threads);
}

void HeKit::AddInplace(seal_gpun::Ciphertext &ct1,
                       const seal_gpun::Ciphertext &ct2) {
  evaluator_->addInplace(ct1, ct2);
}

void HeKit::AddInplace(std::vector<seal_gpun::Ciphertext> &cts1,
                       const std::vector<seal_gpun::Ciphertext> &cts2,
                       bool async, int32_t n_threads) {
  auto eval_func = std::bind(&seal_gpun::Evaluator::addInplace, evaluator_,
                             std::placeholders::_1, std::placeholders::_2);
  EvalInplace(cts1, cts2, eval_func, async, n_threads);
}

void HeKit::Sub(const seal_gpun::Ciphertext &ct1,
                const seal_gpun::Ciphertext &ct2, seal_gpun::Ciphertext &out) {
  evaluator_->sub(ct1, ct2, out);
}

void HeKit::Sub(const std::vector<seal_gpun::Ciphertext> &cts1,
                const std::vector<seal_gpun::Ciphertext> &cts2,
                std::vector<seal_gpun::Ciphertext> &out, bool async,
                int32_t n_threads) {
  auto eval_func =
      std::bind(&seal_gpun::Evaluator::sub, evaluator_, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  Eval(cts1, cts2, out, eval_func, async, n_threads);
}

void HeKit::SubInplace(seal_gpun::Ciphertext &ct1,
                       const seal_gpun::Ciphertext &ct2) {
  evaluator_->subInplace(ct1, ct2);
}

void HeKit::SubInplace(std::vector<seal_gpun::Ciphertext> &cts1,
                       const std::vector<seal_gpun::Ciphertext> &cts2,
                       bool async, int32_t n_threads) {
  auto eval_func = std::bind(&seal_gpun::Evaluator::subInplace, evaluator_,
                             std::placeholders::_1, std::placeholders::_2);
  EvalInplace(cts1, cts2, eval_func, async, n_threads);
}

template void HeKit::Encode(const std::vector<int64_t> &ms,
                            std::vector<seal_gpun::Plaintext> &out, bool async,
                            int32_t n_threads);
template void HeKit::Encode(const std::vector<double> &ms,
                            std::vector<seal_gpun::Plaintext> &out, bool async,
                            int32_t n_threads);

template void HeKit::Encrypt(const int64_t m, seal_gpun::Ciphertext &out);
template void HeKit::Encrypt(const double m, seal_gpun::Ciphertext &out);

template void HeKit::Encrypt(const std::vector<int64_t> &pts,
                             std::vector<seal_gpun::Ciphertext> &out,
                             bool async, int32_t n_threads);
template void HeKit::Encrypt(const std::vector<double> &pts,
                             std::vector<seal_gpun::Ciphertext> &out,
                             bool async, int32_t n_threads);
template void HeKit::Encrypt(const std::vector<seal_gpun::Plaintext> &pts,
                             std::vector<seal_gpun::Ciphertext> &out,
                             bool async, int32_t n_threads);
//==========================fhe_gpu operation end============================
}  // namespace heu::algos::seal_fhe::gpu
