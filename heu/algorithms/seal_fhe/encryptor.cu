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

#include "heu/algorithms/seal_fhe/encryptor.cuh"

#include <string>

#include "fmt/ranges.h"

namespace heu::algos::seal_fhe {

Encryptor::Encryptor(const SEALContext &context, const PublicKey &pk) {
  encryptor_ = std::make_shared<seal_gpun::Encryptor>(context, pk);
}

Encryptor::Encryptor(const std::shared_ptr<seal_gpun::Encryptor> &encryptor)
    : encryptor_(encryptor) {}

Ciphertext Encryptor::EncryptZeroT() const { return encryptor_->encryptZero(); }

Ciphertext Encryptor::Encrypt(const Plaintext &m) const {
  return encryptor_->encrypt(m);
}

Ciphertext Encryptor::SemiEncrypt(const Plaintext &plaintext) const {
  return encryptor_->encryptSymmetric(plaintext);
}

void Encryptor::Encrypt(const Plaintext &m, Ciphertext *out) const {
  encryptor_->encrypt(m, *out);
}

void Encryptor::EncryptWithAudit(const Plaintext &m, Ciphertext *ct_out,
                                 std::string *audit_out) const {
  encryptor_->encrypt(m, *ct_out);
  audit_out->assign(fmt::format("mock_fhe:{}", m.to_string()));
}

}  // namespace heu::algos::seal_fhe
