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

#pragma once

#include "heu/library/algorithms/paillier_float/ciphertext.h"
#include "heu/library/algorithms/paillier_float/public_key.h"
#include "heu/library/algorithms/paillier_float/secret_key.h"

namespace heu::lib::algorithms::paillier_f {

class Decryptor {
 public:
  explicit Decryptor(PublicKey pk, SecretKey sk)
      : pk_(std::move(pk)), sk_(std::move(sk)) {}

  void Decrypt(const Ciphertext& cipher, MPInt* plain) const;
  MPInt Decrypt(const Ciphertext& cipher) const;

  void Decrypt(const Ciphertext& cipher, double* plain) const;

  const SecretKey& secret_key() const { return sk_; }

 private:
  void DecryptRaw(const MPInt& c, MPInt* m) const;

 private:
  PublicKey pk_;
  SecretKey sk_;
};

}  // namespace heu::lib::algorithms::paillier_f
