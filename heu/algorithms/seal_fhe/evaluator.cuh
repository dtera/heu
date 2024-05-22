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

#include <cstdint>
#include <memory>

#include "heu/algorithms/seal_fhe/base.cuh"
#include "heu/spi/he/schema.h"
#include "heu/spi/he/sketches/scalar/word_evaluator.h"

namespace heu::algos::seal_fhe {

class Evaluator : public spi::WordEvaluatorScalarSketch<Plaintext, Ciphertext> {
 public:
  Evaluator(const spi::Schema &schema, const SEALContext &context,
            const seal_gpun::RelinKeys &relinKeys,
            const seal_gpun::GaloisKeys &galoisKeys);

  Evaluator(const spi::Schema &schema,
            const std::shared_ptr<seal_gpun::Evaluator> &evaluator,
            const seal_gpun::RelinKeys &relinKeys,
            const seal_gpun::GaloisKeys &galoisKeys);

  Plaintext Negate(const Plaintext &a) const override;
  void NegateInplace(Plaintext *a) const override;
  Ciphertext Negate(const Ciphertext &a) const override;
  void NegateInplace(Ciphertext *a) const override;

  Plaintext Add(const Plaintext &a, const Plaintext &b) const override;
  Ciphertext Add(const Ciphertext &a, const Plaintext &b) const override;
  Ciphertext Add(const Ciphertext &a, const Ciphertext &b) const override;
  void AddInplace(Ciphertext *a, const Plaintext &b) const override;
  void AddInplace(Ciphertext *a, const Ciphertext &b) const override;

  Plaintext Mul(const Plaintext &a, const Plaintext &b) const override;
  Ciphertext Mul(const Ciphertext &a, const Plaintext &b) const override;
  Ciphertext Mul(const Ciphertext &a, const Ciphertext &b) const override;
  void MulInplace(Ciphertext *a, const Plaintext &b) const override;
  void MulInplace(Ciphertext *a, const Ciphertext &b) const override;

  Plaintext Square(const Plaintext &a) const override;
  Ciphertext Square(const Ciphertext &a) const override;
  void SquareInplace(Plaintext *a) const override;
  void SquareInplace(Ciphertext *a) const override;

  Plaintext Pow(const Plaintext &a, int64_t exponent) const override;
  Ciphertext Pow(const Ciphertext &a, int64_t exponent) const override;
  void PowInplace(Plaintext *a, int64_t exponent) const override;
  void PowInplace(Ciphertext *a, int64_t exponent) const override;

  void Randomize(Ciphertext *ct) const override;

  Ciphertext Relinearize(const Ciphertext &a) const override;
  void RelinearizeInplace(Ciphertext *a) const override;

  Ciphertext ModSwitch(const Ciphertext &a) const override;
  void ModSwitchInplace(Ciphertext *a) const override;

  Ciphertext Rescale(const Ciphertext &a) const override;
  void RescaleInplace(Ciphertext *a) const override;

  Ciphertext SwapRows(const Ciphertext &a) const override;
  void SwapRowsInplace(Ciphertext *a) const override;

  Ciphertext Conjugate(const Ciphertext &a) const override;
  void ConjugateInplace(Ciphertext *a) const override;

  Ciphertext Rotate(const Ciphertext &a, int steps) const override;
  void RotateInplace(Ciphertext *a, int steps) const override;

  void BootstrapInplace(Ciphertext *a) const override;

 private:
  template <typename T>
  void DoPow(const T &a, int64_t exp, T *out) const;

  spi::Schema schema_;
  std::shared_ptr<seal_gpun::Evaluator> evaluator_;
  seal_gpun::RelinKeys relinKeys_;
  seal_gpun::GaloisKeys galoisKeys_;
};

}  // namespace heu::algos::seal_fhe
