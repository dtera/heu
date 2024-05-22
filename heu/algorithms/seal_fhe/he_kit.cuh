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

#include <memory>
#include <string>

#include "heu/algorithms/seal_fhe/base.cuh"
#include "heu/spi/he/encoder.h"
#include "heu/spi/he/sketches/common/he_kit.h"

namespace heu::algos::seal_fhe {

class HeKit : public spi::HeKitSketch<SecretKey, PublicKey, RelinKeys,
                                      GaloisKeys, BootstrapKey> {
 public:
  std::string GetLibraryName() const override;
  spi::Schema GetSchema() const override;
  spi::FeatureSet GetFeatureSet() const override;

  std::string ToString() const override;

  size_t Serialize(uint8_t *buf, size_t buf_len) const override;
  size_t Serialize(spi::HeKeyType key_type, uint8_t *buf,
                   size_t buf_len) const override;

  static std::unique_ptr<spi::HeKit> Create(spi::Schema schema,
                                            const spi::SpiArgs &args);

  static bool Check(spi::Schema schema, const spi::SpiArgs &);

  inline const std::shared_ptr<seal_gpun::Encryptor> &GetCudaEncryptor() {
    return encryptor_cuda_;
  }

  inline const std::shared_ptr<seal_gpun::Decryptor> &GetCudaDecryptor() {
    return decryptor_cuda_;
  }

  inline const std::shared_ptr<seal_gpun::Evaluator> &GetCudaEvaluator() {
    return evaluator_cuda_;
  }

  inline const std::shared_ptr<seal_gpun::CKKSEncoder> &GetCudaCKKSEncoder() {
    return ckks_encoder_cuda_;
  }

  inline const std::shared_ptr<seal_gpun::BatchEncoder> &GetCudaBatchEncoder() {
    return batch_encoder_cuda_;
  }

 protected:
  std::shared_ptr<spi::Encoder> CreateEncoder(
      const yacl::SpiArgs &args) const override;

 private:
  void InitOperators();
  void Deserialize(yacl::ByteContainerView in);

  spi::Schema schema_;
  int64_t scale_ = 1L << 40;
  std::shared_ptr<SEALContext> context_;
  std::shared_ptr<seal_gpun::Encryptor> encryptor_cuda_;
  std::shared_ptr<seal_gpun::Decryptor> decryptor_cuda_;
  std::shared_ptr<seal_gpun::Evaluator> evaluator_cuda_;
  std::shared_ptr<seal_gpun::CKKSEncoder> ckks_encoder_cuda_;
  std::shared_ptr<seal_gpun::BatchEncoder> batch_encoder_cuda_;
};

}  // namespace heu::algos::seal_fhe
