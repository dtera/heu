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

#include "absl/types/optional.h"

#include "heu/library/algorithms/paillier_float/ciphertext.h"
#include "heu/library/algorithms/paillier_float/internal/codec.h"
#include "heu/library/algorithms/paillier_float/public_key.h"

namespace heu::lib::algorithms::paillier_f {

// Forward declaration
class Evaluator;

class Encryptor {
  friend class Evaluator;

 public:
  explicit Encryptor(PublicKey pk) : pk_(std::move(pk)) {}

  // get ciphertext rep of zero
  Ciphertext EncryptZero() const;

  Ciphertext Encrypt(int64_t plain) const;

  Ciphertext Encrypt(const MPInt &plain) const;

  Ciphertext Encrypt(double plain,
                     absl::optional<float> precision = absl::nullopt) const;

  std::pair<Ciphertext, std::string> EncryptWithAudit(const MPInt &m) const;

 public:
  const PublicKey &public_key() const { return pk_; }

 private:
  /**
   * @params rand: obfuscator for the ciphertext, default value is
   * `absl::nullopt`, a random value generated by MPInt::RandomLtN() will be
   * used
   */
  Ciphertext EncryptEncoded(
      const internal::EncodedNumber &encoding,
      absl::optional<uint32_t> rand = absl::nullopt) const;

  template <bool audit = false>
  MPInt EncryptRaw(const MPInt &m,
                   absl::optional<uint32_t> rand = absl::nullopt,
                   std::string *audit_str = nullptr) const;

 private:
  PublicKey pk_;
};

}  // namespace heu::lib::algorithms::paillier_f