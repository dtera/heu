// Copyright 2024 Ant Group Co., Ltd.
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
#include <vector>

#include "heu/library/algorithms/seal_gpu/seal_cuda.cuh"
#include "heu/spi/he/sketches/common/keys.h"
#include "heu/spi/he/sketches/scalar/item_tool.h"

using seal_gpun::Ciphertext;
using seal_gpun::Plaintext;
using seal_gpun::SEALContext;

namespace heu::algos::seal_fhe {

class SecretKey : public spi::KeySketch<spi::HeKeyType::SecretKey>,
                  public seal_gpun::SecretKey {
 public:
  SecretKey() = default;

  SecretKey(const seal_gpun::SecretKey &secretKey)
      : seal_gpun::SecretKey(secretKey) {}

  std::map<std::string, std::string> ListParams() const override {
    auto sk = data();
    return {
        {"is_ntt_form_", fmt::to_string(sk.isNttForm())},
        {"coeff_count_", fmt::to_string(sk.coeffCount())},
        {"scale_", fmt::to_string(sk.scale())},
    };
  }
};

class PublicKey : public spi::KeySketch<spi::HeKeyType::PublicKey>,
                  public seal_gpun::PublicKey {
 public:
  std::map<std::string, std::string> ListParams() const override {
    auto pk = data();
    return {
        {"is_ntt_form_", fmt::to_string(pk.isNttForm())},
        {"size_", fmt::to_string(pk.size())},
        {"poly_modulus_degree_", fmt::to_string(pk.polyModulusDegree())},
        {"coeff_modulus_size_", fmt::to_string(pk.coeffModulusSize())},
        {"scale_", fmt::to_string(pk.scale())},
        {"correction_factor_", fmt::to_string(pk.correctionFactor())},
        {"seed_", fmt::to_string(pk.seed())},
    };
  }
};

class RelinKeys : public spi::KeySketch<spi::HeKeyType::RelinKeys>,
                  public seal_gpun::RelinKeys {
 public:
  std::map<std::string, std::string> ListParams() const override {
    return {
        {"num_of_keyswitch_", fmt::to_string(size())},
    };
  }
};

class GaloisKeys : public spi::KeySketch<spi::HeKeyType::GaloisKeys>,
                   public seal_gpun::GaloisKeys {
 public:
  std::map<std::string, std::string> ListParams() const override {
    return {
        {"num_of_keyswitch_", fmt::to_string(size())},
    };
  }
};

class BootstrapKey : public spi::EmptyKeySketch<spi::HeKeyType::BootstrapKey> {
};

class ItemTool : public spi::ItemToolScalarSketch<Plaintext, Ciphertext,
                                                  SecretKey, PublicKey> {
 public:
  Plaintext Clone(const Plaintext &pt) const override;
  Ciphertext Clone(const Ciphertext &ct) const override;

  size_t Serialize(const Plaintext &pt, uint8_t *buf,
                   size_t buf_len) const override;
  size_t Serialize(const Ciphertext &ct, uint8_t *buf,
                   size_t buf_len) const override;

  Plaintext DeserializePT(yacl::ByteContainerView buffer) const override;
  Ciphertext DeserializeCT(yacl::ByteContainerView buffer) const override;
};

}  // namespace heu::algos::seal_fhe
