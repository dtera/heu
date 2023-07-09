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

#include "heu/library/numpy/matrix.h"

namespace heu::lib::numpy {

class Random {
 public:
  static PMatrix RandInt(const phe::Plaintext &min, const phe::Plaintext &max,
                         const Shape &size);

  static PMatrix RandBits(phe::SchemaType schema, size_t bits,
                          const Shape &size);
};

}  // namespace heu::lib::numpy
