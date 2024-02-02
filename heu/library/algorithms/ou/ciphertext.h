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

#include "heu/library/algorithms/util/he_object.h"
#include "heu/library/algorithms/util/mp_int.h"

namespace heu::lib::algorithms::ou {

using Plaintext = MPInt;

class Ciphertext : public HeObject<Ciphertext> {
 public:
  Ciphertext() = default;

  explicit Ciphertext(MPInt c) : c_(std::move(c)) {}

  [[nodiscard]] std::string ToString() const override { return c_.ToString(); }

  bool operator==(const Ciphertext& other) const { return c_ == other.c_; }

  bool operator!=(const Ciphertext& other) const {
    return !this->operator==(other);
  }

#ifdef NO_USE_MSGPACK
  yacl::Buffer Serialize() const override {
    // return c_.ToMagBytes();
    // return c_.Serialize();
    auto n = c_.n_;
    auto size = n.used * 8 + 3;
    std::byte* buf = new std::byte[size];
    buf[0] = static_cast<std::byte>(n.used);
    buf[1] = static_cast<std::byte>(n.alloc);
    buf[2] = static_cast<std::byte>(n.sign);
    // std::memcpy(buf.get() + 3, n.dp, n.used * 8);
    for (int i = 0; i < n.used; ++i) {
      int j = i * 8 + 3;
      buf[j] = static_cast<std::byte>(n.dp[i] >> 56);
      buf[j + 1] = static_cast<std::byte>(n.dp[i] >> 48);
      buf[j + 2] = static_cast<std::byte>(n.dp[i] >> 40);
      buf[j + 3] = static_cast<std::byte>(n.dp[i] >> 32);
      buf[j + 4] = static_cast<std::byte>(n.dp[i] >> 24);
      buf[j + 5] = static_cast<std::byte>(n.dp[i] >> 16);
      buf[j + 6] = static_cast<std::byte>(n.dp[i] >> 8);
      buf[j + 7] = static_cast<std::byte>(n.dp[i]);
    }
    return yacl::Buffer(buf, size, [](void* ptr) { free(ptr); });
  }

  void Deserialize(yacl::ByteContainerView in) override {
    // c_.FromMagBytes(in);
    // c_.Deserialize(in);
    auto ptr = in.data();
    auto size = in.size();
    c_.n_.used = static_cast<int>(ptr[0]);
    c_.n_.alloc = static_cast<int>(ptr[1]);
    c_.n_.sign = static_cast<mp_sign>(ptr[2]);
    ptr += 3;
    c_.n_.dp = new mp_digit[c_.n_.alloc];
    for (int i = 0; i < c_.n_.used; ++i) {
      c_.n_.dp[i] = static_cast<mp_digit>(ptr[0]) << 56 |
                    static_cast<mp_digit>(ptr[1]) << 48 |
                    static_cast<mp_digit>(ptr[2]) << 40 |
                    static_cast<mp_digit>(ptr[3]) << 32 |
                    static_cast<mp_digit>(ptr[4]) << 24 |
                    static_cast<mp_digit>(ptr[5]) << 16 |
                    static_cast<mp_digit>(ptr[6]) << 8 |
                    static_cast<mp_digit>(ptr[7]);
      ptr += 8;
    }
  }
#endif

  MSGPACK_DEFINE(c_);

  // TODO: make this private.
  MPInt c_;
};

}  // namespace heu::lib::algorithms::ou
