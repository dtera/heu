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

#include "heu/algorithms/seal_fhe/encoders.cuh"

namespace heu::algos::seal_fhe {
//==================================//
//             BatchEncoder         //
//==================================//

BatchEncoder::BatchEncoder(const SEALContext &context) {
  batch_encoder_ = std::make_shared<seal_gpun::BatchEncoder>(context);
}

BatchEncoder::BatchEncoder(
    const std::shared_ptr<seal_gpun::BatchEncoder> &batch_encoder)
    : batch_encoder_(batch_encoder) {}

size_t BatchEncoder::SlotCount() const { return batch_encoder_->slotCount(); }

std::string BatchEncoder::ToString() const {
  return fmt::format("BatchEncoder from seal_fhe lib, slot={}", SlotCount());
}

Plaintext BatchEncoder::FromStringT(std::string_view) const {
  YACL_THROW("seal_fhe: directly parse from string not implemented");
}

Plaintext BatchEncoder::EncodeT(absl::Span<const int64_t> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  std::vector<int64_t> vec(SlotCount());
  std::copy(message.begin(), message.end(), vec.begin());
  Plaintext res;
  batch_encoder_->encode(vec, res);
  return res;
}

Plaintext BatchEncoder::EncodeT(absl::Span<const uint64_t> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  std::vector<int64_t> vec(SlotCount());
  std::copy(message.begin(), message.end(), vec.begin());
  Plaintext res;
  batch_encoder_->encode(vec, res);
  return res;
}

Plaintext BatchEncoder::EncodeT(absl::Span<const double> message) const {
  YACL_THROW("BatchEncoder cannot encode float number");
}

Plaintext BatchEncoder::EncodeT(absl::Span<const std::complex<double>>) const {
  YACL_THROW("BatchEncoder cannot encode complex number");
}

Plaintext BatchEncoder::EncodeT(int64_t message) const {
  YACL_THROW("BatchEncode cannot encode a single scalar {}", message);
}

Plaintext BatchEncoder::EncodeT(uint64_t message) const {
  YACL_THROW("BatchEncode cannot encode a single scalar {}", message);
}

Plaintext BatchEncoder::EncodeT(double message) const {
  YACL_THROW("BatchEncode cannot encode a single scalar {}", message);
}

Plaintext BatchEncoder::EncodeT(const std::complex<double> &message) const {
  YACL_THROW("BatchEncode cannot encode a single scalar {}+{}i", message.real(),
             message.imag());
}

void BatchEncoder::DecodeT(const Plaintext &pt, absl::Span<int64_t> out) const {
  YACL_ENFORCE(out.size() >= SlotCount(),
               "Output space is not enough, cannot decode");
  std::vector<int64_t> vec(SlotCount());
  batch_encoder_->decode(pt, vec);
  std::copy(vec.begin(), vec.end(), out.begin());
}

void BatchEncoder::DecodeT(const Plaintext &pt,
                           absl::Span<uint64_t> out) const {
  YACL_ENFORCE(out.size() >= SlotCount(),
               "Output space is not enough, cannot decode");
  std::vector<uint64_t> vec(SlotCount());
  batch_encoder_->decode(pt, vec);
  std::copy(vec.begin(), vec.end(), out.begin());
}

void BatchEncoder::DecodeT(const Plaintext &, absl::Span<double>) const {
  YACL_THROW("BatchEncoder does not support float number");
}

void BatchEncoder::DecodeT(const Plaintext &,
                           absl::Span<std::complex<double>>) const {
  YACL_THROW("BatchEncoder does not support complex number");
}

//==================================//
//             CkksEncoder          //
//==================================//

CkksEncoder::CkksEncoder(const SEALContext &context, const double scale)
    : scale_(scale) {
  ckks_encoder_ = std::make_shared<seal_gpun::CKKSEncoder>(context);
}

CkksEncoder::CkksEncoder(
    const std::shared_ptr<seal_gpun::CKKSEncoder> &ckks_encoder,
    const double scale)
    : scale_(scale), ckks_encoder_(ckks_encoder) {}

size_t CkksEncoder::SlotCount() const { return ckks_encoder_->slotCount(); }

std::string CkksEncoder::ToString() const {
  return fmt::format("CkksEncoder from seal_fhe lib, slot={}", SlotCount());
}

Plaintext CkksEncoder::FromStringT(std::string_view) const {
  YACL_THROW("seal_fhe: directly parse from string not implemented");
}

Plaintext CkksEncoder::EncodeT(absl::Span<const int64_t> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  return Plaintext();
}

Plaintext CkksEncoder::EncodeT(absl::Span<const uint64_t> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  return Plaintext();
}

Plaintext CkksEncoder::EncodeT(absl::Span<const double> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  std::vector<double> vec(SlotCount() * 2);
  vec.assign(message.begin(), message.end());
  Plaintext res;
  ckks_encoder_->encodePolynomial(vec, scale_, res);
  return res;
}

Plaintext CkksEncoder::EncodeT(
    absl::Span<const std::complex<double>> message) const {
  YACL_ENFORCE(message.size() <= SlotCount(), "Illegal input");
  std::vector<std::complex<double>> vec(SlotCount() * 2);
  vec.assign(message.begin(), message.end());
  Plaintext res;
  ckks_encoder_->encode(vec, scale_, res);
  return res;
}

Plaintext CkksEncoder::EncodeT(int64_t message) const {
  Plaintext res;
  ckks_encoder_->encode(message, res);
  return res;
}

Plaintext CkksEncoder::EncodeT(uint64_t message) const {
  Plaintext res;
  ckks_encoder_->encode(message, res);
  return res;
}

Plaintext CkksEncoder::EncodeT(double message) const {
  Plaintext res;
  ckks_encoder_->encode(message, scale_, res);
  return res;
}

Plaintext CkksEncoder::EncodeT(const std::complex<double> &message) const {
  // Encodes a double-precision complex number into a plaintext polynomial.
  // Append zeros to fill all slots.
  Plaintext res;
  ckks_encoder_->encode(message, scale_, res);
  return res;
}

void CkksEncoder::DecodeT(const Plaintext &pt, absl::Span<int64_t> out) const {
  YACL_ENFORCE_GE(out.size(), SlotCount(),
                  "Output space is not enough, cannot decode");
}

void CkksEncoder::DecodeT(const Plaintext &pt, absl::Span<uint64_t> out) const {
  YACL_ENFORCE_GE(out.size(), SlotCount(),
                  "Output space is not enough, cannot decode");
}

void CkksEncoder::DecodeT(const Plaintext &pt, absl::Span<double> out) const {
  YACL_ENFORCE_GE(out.size(), SlotCount(),
                  "Output space is not enough, cannot decode");
  std::vector<double> vec(SlotCount());
  ckks_encoder_->decodePolynomial(pt, vec);
  std::copy(vec.begin(), vec.end(), out.begin());
}

void CkksEncoder::DecodeT(const Plaintext &pt,
                          absl::Span<std::complex<double>> out) const {
  YACL_ENFORCE_GE(out.size(), SlotCount(),
                  "Output space is not enough, cannot decode");
  std::vector<std::complex<double>> vec(SlotCount());
  ckks_encoder_->decode(pt, vec);
  std::copy(vec.begin(), vec.end(), out.begin());
}

}  // namespace heu::algos::seal_fhe
