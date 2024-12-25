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

#include "yacl/utils/serializer.h"

#include "heu/algorithms/common/type_alias.h"
#include "heu/spi/he/sketches/common/keys.h"
#include "heu/spi/he/sketches/common/pt_ct.h"
#include "heu/spi/he/sketches/scalar/item_tool.h"

namespace heu::algos::paillier_z {

using Plaintext = MPInt;

class Ciphertext : spi::PtCtSketch<Ciphertext> {
 public:
  Ciphertext() = default;

  explicit Ciphertext(MPInt c) : c_(std::move(c)) {}

  bool operator==(const Ciphertext &other) const override {
    return c_ == other.c_;
  }

  std::string ToString() const override { return c_.ToHexString(); }

  size_t Serialize(uint8_t *buf, size_t buf_len) const override {
    return c_.Serialize(buf, buf_len);
  }

  void Deserialize(yacl::ByteContainerView buffer) override {
    c_.Deserialize(buffer);
  }

  MPInt c_;
};

class SecretKey : public spi::KeySketch<spi::HeKeyType::SecretKey> {
 public:
  MPInt lambda_;  // lambda = lcm(p−1, q−1)
  MPInt mu_;      // μ = 1 / lambda
  MPInt p_;       // p
  MPInt q_;       // q
  // Node: The following paramters are generated by p & q
  MPInt p_square_;                   // p^2
  MPInt q_square_;                   // q^2
  MPInt n_square_;                   // n_ * n_
  MPInt q_square_inv_mul_q_square_;  // (q^2)^{-1} mod p^2 * q^2
  MPInt p_inv_mod_q_;                // p^{-1} mod q
  MPInt phi_p_square_;               // p(p-1)
  MPInt phi_q_square_;               // q(q-1)
  MPInt phi_p_;                      // p-1
  MPInt phi_q_;                      // q-1
  MPInt hp_;
  MPInt hq_;

  void Init();
  // base^exp mod n^2, n = p * q
  MPInt PowModNSquareCrt(const MPInt &base, const MPInt &exp) const;

  [[nodiscard]] size_t Serialize(uint8_t *buf, size_t buf_len) const {
    return yacl::SerializeVarsTo(buf, buf_len, p_, q_, lambda_, mu_);
  }

  static std::shared_ptr<SecretKey> LoadFrom(yacl::ByteContainerView in) {
    auto sk = std::make_shared<SecretKey>();
    yacl::DeserializeVarsTo(in, &sk->p_, &sk->q_, &sk->lambda_, &sk->mu_);
    sk->Init();
    return sk;
  }

  std::map<std::string, std::string> ListParams() const override {
    return {{"p", p_.ToString()},
            {"q", q_.ToString()},
            {"lambda", lambda_.ToString()},
            {"mu", mu_.ToString()}};
  }
};

class PublicKey : public spi::KeySketch<spi::HeKeyType::PublicKey> {
 public:
  MPInt n_;         // public modulus n = p * q
  MPInt n_square_;  // n_ * n_
  MPInt n_half_;    // n_ / 2
  MPInt h_s_;       // h^n mod n^2

  size_t key_size_;

  std::shared_ptr<MontgomerySpace> m_space_;  // m-space for mod n^2
  std::shared_ptr<BaseTable> hs_table_;       // h_s_ table mod n^2

  // Init pk based on n_
  void Init();

  bool operator==(const PublicKey &other) const {
    return n_ == other.n_ && h_s_ == other.h_s_;
  }

  bool operator!=(const PublicKey &other) const {
    return !this->operator==(other);
  }

  // Valid plaintext range: [n_half_, -n_half]
  [[nodiscard]] inline const MPInt &PlaintextBound() const & { return n_half_; }

  [[nodiscard]] size_t Serialize(uint8_t *buf, size_t buf_len) const {
    return yacl::SerializeVarsTo(buf, buf_len, n_, h_s_);
  }

  static std::shared_ptr<PublicKey> LoadFrom(yacl::ByteContainerView in) {
    auto pk = std::make_shared<PublicKey>();
    yacl::DeserializeVarsTo(in, &pk->n_, &pk->h_s_);
    pk->Init();
    return pk;
  }

  std::map<std::string, std::string> ListParams() const override {
    return {{"key_size", fmt::to_string(key_size_)},
            {"n", n_.ToString()},
            {"h_s", h_s_.ToString()}};
  }
};

class ItemTool : public spi::ItemToolScalarSketch<Plaintext, Ciphertext,
                                                  SecretKey, PublicKey> {
 public:
  Plaintext Clone(const Plaintext &pt) const override;
  Ciphertext Clone(const Ciphertext &ct) const override;
};

}  // namespace heu::algos::paillier_z