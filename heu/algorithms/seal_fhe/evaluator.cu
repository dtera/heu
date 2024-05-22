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

#include "heu/algorithms/seal_fhe/evaluator.cuh"

#include <memory>
#include <vector>

#include "heu/spi/he/he_configs.h"
#include "heu/spi/utils/math_tool.h"

namespace heu::algos::seal_fhe {

Evaluator::Evaluator(const spi::Schema &schema, const SEALContext &context,
                     const seal_gpun::RelinKeys &relinKeys,
                     const seal_gpun::GaloisKeys &galoisKeys)
    : schema_(schema), relinKeys_(relinKeys), galoisKeys_(galoisKeys) {
  evaluator_ = std::make_shared<seal_gpun::Evaluator>(context);
}

Evaluator::Evaluator(const spi::Schema &schema,
                     const std::shared_ptr<seal_gpun::Evaluator> &evaluator,
                     const seal_gpun::RelinKeys &relinKeys,
                     const seal_gpun::GaloisKeys &galoisKeys)
    : schema_(schema),
      evaluator_(evaluator),
      relinKeys_(relinKeys),
      galoisKeys_(galoisKeys) {}

Plaintext Evaluator::Negate(const Plaintext &a) const { return Plaintext(); }

void Evaluator::NegateInplace(Plaintext *a) const {}

Ciphertext Evaluator::Negate(const Ciphertext &a) const {
  Ciphertext res;
  evaluator_->negate(a, res);
  return res;
}

void Evaluator::NegateInplace(Ciphertext *a) const {
  evaluator_->negateInplace(*a);
}

Plaintext Evaluator::Add(const Plaintext &a, const Plaintext &b) const {
  return Plaintext();
}

Ciphertext Evaluator::Add(const Ciphertext &a, const Plaintext &b) const {
  Ciphertext res;
  evaluator_->addPlain(a, b, res);
  return res;
}

Ciphertext Evaluator::Add(const Ciphertext &a, const Ciphertext &b) const {
  Ciphertext res;
  evaluator_->add(a, b, res);
  return res;
}

void Evaluator::AddInplace(Ciphertext *a, const Plaintext &b) const {
  evaluator_->addPlainInplace(*a, b);
}

void Evaluator::AddInplace(Ciphertext *a, const Ciphertext &b) const {
  evaluator_->addInplace(*a, b);
}

Plaintext Evaluator::Mul(const Plaintext &a, const Plaintext &b) const {
  return Plaintext();
}

Ciphertext Evaluator::Mul(const Ciphertext &a, const Plaintext &b) const {
  Ciphertext res;
  evaluator_->multiplyPlain(a, b, res);
  return res;
}

Ciphertext Evaluator::Mul(const Ciphertext &a, const Ciphertext &b) const {
  Ciphertext res;
  evaluator_->multiply(a, b, res);
  return res;
}

void Evaluator::MulInplace(Ciphertext *a, const Plaintext &b) const {
  evaluator_->multiplyPlainInplace(*a, b);
}

void Evaluator::MulInplace(Ciphertext *a, const Ciphertext &b) const {
  evaluator_->multiplyInplace(*a, b);
}

Plaintext Evaluator::Square(const Plaintext &a) const { return Mul(a, a); }

Ciphertext Evaluator::Square(const Ciphertext &a) const {
  Ciphertext res;
  evaluator_->square(a, res);
  return res;
}

void Evaluator::SquareInplace(Plaintext *a) const { *a = Square(*a); }

void Evaluator::SquareInplace(Ciphertext *a) const {
  evaluator_->squareInplace(*a);
}

template <typename T>
void Evaluator::DoPow(const T &a, int64_t exp, T *out) const {
  bool first = true;
  T s = a;
  while (exp != 0) {
    if (exp & 1) {
      if (first) {
        *out = s;
        first = false;
      } else {
        MulInplace(out, s);
      }
    }
    exp >>= 1;
    if (exp != 0) {
      MulInplace(&s, s);
    }
  }
}

Plaintext Evaluator::Pow(const Plaintext &a, int64_t exponent) const {
  Plaintext res;
  // DoPow(a, exponent, &res);
  return Plaintext();
}

Ciphertext Evaluator::Pow(const Ciphertext &a, int64_t exponent) const {
  Ciphertext res;
  DoPow(a, exponent, &res);
  return res;
}

void Evaluator::PowInplace(Plaintext *a, int64_t exponent) const {
  // DoPow(*a, exponent, a);
}

void Evaluator::PowInplace(Ciphertext *a, int64_t exponent) const {
  DoPow(*a, exponent, a);
}

void Evaluator::Randomize(Ciphertext *) const {
  // nothing to do
}

Ciphertext Evaluator::Relinearize(const Ciphertext &a) const {
  Ciphertext res;
  evaluator_->relinearize(a, relinKeys_, res);
  return res;
}

void Evaluator::RelinearizeInplace(Ciphertext *a) const {
  evaluator_->relinearizeInplace(*a, relinKeys_);
}

Ciphertext Evaluator::ModSwitch(const Ciphertext &a) const {
  return schema_ == spi::Schema::GPU_CKKS ? Rescale(a) : a;
}

void Evaluator::ModSwitchInplace(Ciphertext *a) const {
  if (schema_ == spi::Schema::GPU_CKKS) {
    RescaleInplace(a);
  }
}

Ciphertext Evaluator::Rescale(const Ciphertext &a) const {
  YACL_ENFORCE(schema_ == spi::Schema::GPU_CKKS,
               "Only gpu_ckks algo supports rescale");
  Ciphertext res;
  evaluator_->rescaleToNext(a, res);
  return res;
}

void Evaluator::RescaleInplace(Ciphertext *a) const {
  YACL_ENFORCE(schema_ == spi::Schema::GPU_CKKS,
               "Only gpu_ckks algo supports rescale");
  evaluator_->rescaleToNextInplace(*a);
}

Ciphertext Evaluator::SwapRows(const Ciphertext &a) const {
  YACL_ENFORCE(schema_ == spi::Schema::GPU_BFV,
               "Only bfv and bgv schema can swap rows");

  Ciphertext res;
  return res;
}

void Evaluator::SwapRowsInplace(Ciphertext *a) const {
  YACL_ENFORCE(schema_ == spi::Schema::GPU_BFV,
               "Only bfv and bgv schema can swap rows");
}

Ciphertext Evaluator::Conjugate(const Ciphertext &a) const {
  Ciphertext res;
  evaluator_->complexConjugate(a, galoisKeys_, res);
  return res;
}

void Evaluator::ConjugateInplace(Ciphertext *a) const {
  YACL_ENFORCE(schema_ == spi::Schema::GPU_CKKS,
               "Only ckks supports conjugate");
  evaluator_->complexConjugateInplace(*a, galoisKeys_);
}

// rotates the vector cyclically to the left (steps > 0) or to the right (steps
// < 0).
Ciphertext Evaluator::Rotate(const Ciphertext &a, int steps) const {
  Ciphertext res;
  evaluator_->rotateRows(a, steps, galoisKeys_, res);
  return res;
}

void Evaluator::RotateInplace(Ciphertext *a, int steps) const {
  evaluator_->rotateRowsInplace(*a, steps, galoisKeys_);
}

void Evaluator::BootstrapInplace(Ciphertext *) const {}

}  // namespace heu::algos::seal_fhe
