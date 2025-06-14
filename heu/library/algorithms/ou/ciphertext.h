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

#include "heu/library/algorithms/util/big_int.h"
#include "heu/library/algorithms/util/he_object.h"

namespace heu::lib::algorithms::ou {

using Plaintext = BigInt;

class Ciphertext : public HeObject<Ciphertext> {
 public:
  Ciphertext() = default;

  explicit Ciphertext(BigInt c) : c_(std::move(c)) {}

  [[nodiscard]] std::string ToString() const override { return c_.ToString(); }

  bool operator==(const Ciphertext &other) const { return c_ == other.c_; }

  bool operator!=(const Ciphertext &other) const {
    return !this->operator==(other);
  }

  yacl::Buffer Serialize() const override {
    // return c_.ToMagBytes();
    return c_.Serialize();
  }

  void Deserialize(yacl::ByteContainerView in) override {
    // c_.FromMagBytes(in);
    c_.Deserialize(in);
  }

  MSGPACK_DEFINE(c_);

  // TODO: make this private.
  BigInt c_;
};

}  // namespace heu::lib::algorithms::ou
