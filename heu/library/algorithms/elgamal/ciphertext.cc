// Copyright 2023 Ant Group Co., Ltd.
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

#include "heu/library/algorithms/elgamal/ciphertext.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace heu::lib::algorithms::elgamal {

namespace {
std::unordered_map<size_t, std::shared_ptr<yacl::crypto::EcGroup>>
    kEcGroupCache;

size_t HashEcGroup(const std::shared_ptr<yacl::crypto::EcGroup>& ec) {
  auto h = std::hash<std::string>();
  return h(ec->GetCurveName()) ^ h(ec->GetLibraryName());
}

}  // namespace

std::string Ciphertext::ToString() const {
  return fmt::format("ElGamal ciphertext {{c1={}, c2={}}}",
                     ec->GetAffinePoint(c1), ec->GetAffinePoint(c2));
}

std::ostream& operator<<(std::ostream& os, const Ciphertext& c) {
  return os << c.ToString();
}

bool Ciphertext::operator==(const Ciphertext& other) const {
  return ec && other.ec && ec->PointEqual(c1, other.c1) &&
         ec->PointEqual(c2, other.c2);
}

bool Ciphertext::operator!=(const Ciphertext& other) const {
  return !(*this == other);
}

void Ciphertext::EnableEcGroup(
    const std::shared_ptr<yacl::crypto::EcGroup>& curve) {
  static std::mutex m;
  std::lock_guard<std::mutex> lock(m);
  kEcGroupCache.try_emplace(HashEcGroup(curve), curve);
}

yacl::Buffer Ciphertext::Serialize(
#ifndef NO_USE_MSGPACK
    bool with_meta
#endif
) const {
#ifndef NO_USE_MSGPACK
  msgpack::sbuffer buffer;
  msgpack::packer<msgpack::sbuffer> o(buffer);

  o.pack_array(with_meta ? 4 : 3);
  if (with_meta) {
    o.pack(ec->GetCurveName());
    o.pack(ec->GetLibraryName());
  } else {
    o.pack(HashEcGroup(ec));
  }
  o.pack(std::string_view(ec->SerializePoint(c1)));
  o.pack(std::string_view(ec->SerializePoint(c2)));

  auto sz = buffer.size();
  return {buffer.release(), sz, [](void* ptr) { free(ptr); }};
#else
  auto ca1 = std::get<yacl::crypto::Array160>(c1);
  auto ca2 = std::get<yacl::crypto::Array160>(c2);
  auto size = ca1.size();
  auto hash_size = sizeof(size_t);
  yacl::Buffer buf(size * 2 + hash_size);
  std::byte* data = static_cast<std::byte*>(buf.data());
  // std::memcpy(ptr_ + size, buf2.data(), size);
  std::memcpy(data, ca1.data(), size);
  std::memcpy(data + size, ca2.data(), size);
  auto hash = HashEcGroup(ec);
  std::memcpy(data + 2 * size, &hash, hash_size);
  return buf;
#endif
}

void Ciphertext::Deserialize(yacl::ByteContainerView in) {
#ifndef NO_USE_MSGPACK
  auto msg =
      msgpack::unpack(reinterpret_cast<const char*>(in.data()), in.size());
  msgpack::object object = msg.get();

  if (object.type != msgpack::type::ARRAY) {
    throw msgpack::type_error();
  }
  if (object.via.array.size != 4 && object.via.array.size != 3) {
    throw msgpack::type_error();
  }

  int idx = 0;
  if (object.via.array.size == 4) {
    auto curve_name = object.via.array.ptr[idx++].as<yacl::crypto::CurveName>();
    auto lib_name = object.via.array.ptr[idx++].as<std::string>();
    ec = ::yacl::crypto::EcGroupFactory::Instance().Create(
        curve_name, yacl::ArgLib = lib_name);
    EnableEcGroup(ec);
  } else {
    auto hash = object.via.array.ptr[idx++].as<size_t>();
    ec = kEcGroupCache.at(hash);
  }

  c1 = ec->DeserializePoint(object.via.array.ptr[idx++].as<std::string_view>());
  c2 = ec->DeserializePoint(object.via.array.ptr[idx++].as<std::string_view>());
#else
  auto hash_size = sizeof(size_t);
  auto size = (in.size() - hash_size) / 2;
  yacl::crypto::Array160 a1, a2;
  std::memcpy(a1.data(), in.data(), size);
  std::memcpy(a2.data(), in.data() + size, size);
  c1 = std::move(a1);
  c2 = std::move(a2);
  std::size_t hash;
  std::memcpy(&hash, in.data() + 2 * size, hash_size);
  ec = kEcGroupCache.at(hash);
#endif
}

}  // namespace heu::lib::algorithms::elgamal
