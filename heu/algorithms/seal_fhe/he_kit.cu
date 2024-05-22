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

#include <memory>
#include <string>

#include "yacl/utils/serializer.h"

#include "heu/algorithms/common/type_alias.h"
#include "heu/algorithms/seal_fhe/decryptor.cuh"
#include "heu/algorithms/seal_fhe/encoders.cuh"
#include "heu/algorithms/seal_fhe/encryptor.cuh"
#include "heu/algorithms/seal_fhe/evaluator.cuh"
#include "heu/algorithms/seal_fhe/he_kit.cuh"
#include "heu/spi/he/he.h"
#include "heu/spi/utils/math_tool.h"

namespace heu::algos::seal_fhe {

static const std::string kLibName = "seal_fhe";  // do not change

std::string HeKit::GetLibraryName() const { return kLibName; }

spi::Schema HeKit::GetSchema() const { return schema_; }

spi::FeatureSet HeKit::GetFeatureSet() const {
  return schema_ == spi::Schema::GPU_CKKS ? spi::FeatureSet::ApproxFHE
                                          : spi::FeatureSet::WordFHE;
}

std::string HeKit::ToString() const {
  if (schema_ == spi::Schema::GPU_CKKS) {
    return fmt::format("{} schema from {} lib, poly_degree={}, scale={}",
                       GetSchema(), GetLibraryName(),
                       context_->keyContextData()->parms().polyModulusDegree(),
                       scale_);
  } else {
    return fmt::format("{} schema from {} lib, poly_degree={}", GetSchema(),
                       GetLibraryName(),
                       context_->keyContextData()->parms().polyModulusDegree());
  }
}

size_t HeKit::Serialize(uint8_t *buf, size_t len) const {
  return yacl::SerializeVarsTo(buf, len, spi::Schema2String(schema_), scale_);
}

void HeKit::Deserialize(yacl::ByteContainerView in) {
  std::string schema_str;
  yacl::DeserializeVarsTo(in, &schema_str, &scale_);
  schema_ = spi::String2Schema(schema_str);
}

size_t HeKit::Serialize(spi::HeKeyType key_type, uint8_t *buf,
                        size_t len) const {
  return Serialize(buf, len);
}

bool HeKit::Check(spi::Schema schema, const spi::SpiArgs &) {
  return schema == spi::Schema::GPU_BFV || schema == spi::Schema::GPU_CKKS;
}

std::shared_ptr<spi::Encoder> HeKit::CreateEncoder(
    const yacl::SpiArgs &args) const {
  if (args.Exist(spi::ArgScale)) {
    YACL_ENFORCE(schema_ != spi::Schema::GPU_BFV,
                 "gpu_bfv schema do not support scale arg.");
    YACL_ENFORCE(args.GetRequired(spi::ArgScale) == scale_,
                 "gpu_ckks: You shouldn't change scale, scale is already set "
                 "in factory");
  }

  if (schema_ == spi::Schema::GPU_BFV) {
    // return std::make_shared<BatchEncoder>(*context_);
    return std::make_shared<BatchEncoder>(batch_encoder_cuda_);
  } else {
    // return std::make_shared<CkksEncoder>(*context_, scale_);
    return std::make_shared<CkksEncoder>(ckks_encoder_cuda_, scale_);
  }
}

std::unique_ptr<spi::HeKit> HeKit::Create(spi::Schema schema,
                                          const spi::SpiArgs &args) {
  YACL_ENFORCE(
      schema == spi::Schema::GPU_BFV || schema == spi::Schema::GPU_CKKS,
      "Schema {} not supported by {}", schema, kLibName);
  YACL_ENFORCE(
      args.Exist(spi::ArgGenNewPkSk) || args.Exist(spi::ArgPkFrom),
      "Neither ArgGenNewPkSk nor ArgPkFrom is set, you must set one of them");

  // process context_
  auto kit = std::make_unique<HeKit>();
  kit->schema_ = schema;
  // first deserialize previous context_
  if (args.Exist(spi::ArgParamsFrom)) {
    kit->Deserialize(args.GetRequired(spi::ArgParamsFrom));
  }
  // next, if the user specifies arguments, override the previous context_.
  auto poly_modulus_degree = args.GetOrDefault(spi::ArgPolyModulusDegree, 4096);
  YACL_ENFORCE(spi::utils::IsPowerOf2(poly_modulus_degree),
               "Poly degree {} must be power of two", poly_modulus_degree);

  if (schema == spi::Schema::GPU_CKKS) {
    kit->scale_ = args.GetOrDefault(spi::ArgScale, kit->scale_);
    YACL_ENFORCE(kit->scale_ != 0, "scale must not be zero");
  }
  std::vector<int> bit_sizes =
      args.GetOrDefault(spi::ArgBitSizes, {60, 60, 60});

  seal_gpu::EncryptionParameters param(schema == spi::Schema::GPU_CKKS
                                           ? seal_gpu::SchemeType::ckks
                                           : seal_gpu::SchemeType::bfv);
  param.setPolyModulusDegree(poly_modulus_degree);
  param.setCoeffModulus(
      seal_gpu::CoeffModulus::Create(poly_modulus_degree, bit_sizes));
  kit->context_ = std::make_shared<SEALContext>(param);

  // process keys
  kit->pk_ = std::make_shared<PublicKey>();
  if (args.GetOptional(spi::ArgGenNewPkSk)) {
    // generate new keys
    auto kg = new seal_gpun::KeyGenerator(*kit->context_);
    kit->sk_ = std::make_shared<SecretKey>(kg->secretKey());
    if (args.GetOrDefault(spi::ArgGenNewRlk, false)) {
      kit->rlk_ = std::make_shared<RelinKeys>();
    }
    if (args.GetOrDefault(spi::ArgGenNewGlk, false)) {
      kit->glk_ = std::make_shared<GaloisKeys>();
    }
    if (args.GetOrDefault(spi::ArgGenNewBsk, false)) {
      kit->bsk_ = std::make_shared<BootstrapKey>();
    }

    kg->createPublicKey(*kit->pk_);
    kg->createRelinKeys(*kit->rlk_);
    kg->createGaloisKeys(*kit->glk_);
  } else {
    // recover all keys from buffer
    if (args.Exist(spi::ArgSkFrom)) {
      kit->sk_ = std::make_shared<SecretKey>();
    }
    YACL_ENFORCE(args.Exist(spi::ArgPkFrom),
                 "no public key buffer found, cannot deserialize");
    if (args.Exist(spi::ArgRlkFrom)) {
      kit->rlk_ = std::make_shared<RelinKeys>();
    }
    if (args.Exist(spi::ArgGlkFrom)) {
      kit->glk_ = std::make_shared<GaloisKeys>();
    }
    if (args.Exist(spi::ArgBskFrom)) {
      kit->bsk_ = std::make_shared<BootstrapKey>();
    }
  }

  kit->InitOperators();
  return kit;
}

void HeKit::InitOperators() {
  encryptor_cuda_ = std::make_shared<seal_gpun::Encryptor>(*context_, *pk_);
  decryptor_cuda_ = std::make_shared<seal_gpun::Decryptor>(*context_, *sk_);
  evaluator_cuda_ = std::make_shared<seal_gpun::Evaluator>(*context_);
  ckks_encoder_cuda_ = std::make_shared<seal_gpun::CKKSEncoder>(*context_);
  batch_encoder_cuda_ = std::make_shared<seal_gpun::BatchEncoder>(*context_);

  item_tool_ = std::make_shared<ItemTool>();
  encryptor_ = std::make_shared<Encryptor>(encryptor_cuda_);
  if (sk_) {
    decryptor_ = std::make_shared<Decryptor>(decryptor_cuda_);
  }
  word_evaluator_ =
      std::make_shared<Evaluator>(schema_, evaluator_cuda_, *rlk_, *glk_);
  /*encryptor_ = std::make_shared<Encryptor>(*context_, *pk_);
  if (sk_) {
    decryptor_ = std::make_shared<Decryptor>(*context_, *sk_);
  }
  word_evaluator_ =
      std::make_shared<Evaluator>(schema_, *context_, *rlk_, *glk_);*/
}

REGISTER_HE_LIBRARY(kLibName, 1, HeKit::Check, HeKit::Create);

}  // namespace heu::algos::seal_fhe
