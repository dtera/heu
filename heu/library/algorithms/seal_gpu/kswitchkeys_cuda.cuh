// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kswitchkeys.h"
#include "publickey_cuda.cuh"

namespace seal_gpu {
/**
Class to store keyswitching keys. It should never be necessary for normal
users to create an instance of KSwitchKeys. This class is used strictly as
a base class for RelinKeys and GaloisKeys classes.

@par Keyswitching
Concretely, keyswitching is used to change a ciphertext encrypted with one
key to be encrypted with another key. It is a general technique and is used
in relinearization and Galois rotations. A keyswitching key contains a sequence
(vector) of keys. In RelinKeys, each key is an encryption of a power of the
secret key. In GaloisKeys, each key corresponds to a type of rotation.

@par Thread Safety
In general, reading from KSwitchKeys is thread-safe as long as no
other thread is concurrently mutating it. This is due to the underlying
data structure storing the keyswitching keys not being thread-safe.

@see RelinKeys for the class that stores the relinearization keys.
@see GaloisKeys for the class that stores the Galois keys.
*/
class KSwitchKeysCuda {
  friend class KeyGeneratorCuda;
  friend class RelinKeysCuda;
  friend class GaloisKeysCuda;

 public:
  /**
  Creates an empty KSwitchKeys.
  */
  KSwitchKeysCuda() = default;

  KSwitchKeysCuda(const KSwitchKeys &k) {
    const auto &kd = k.data();
    keys_.clear();
    keys_.reserve(kd.size());
    for (size_t i = 0; i < kd.size(); i++) {
      std::vector<PublicKeyCuda> cur;
      cur.reserve(kd[i].size());
      for (size_t j = 0; j < kd[i].size(); j++) {
        cur.push_back(PublicKeyCuda(kd[i][j]));
      }
      keys_.push_back(std::move(cur));
    }
    parms_id_ = k.parmsID();
  }

  /**
  Creates a new KSwitchKeys instance by copying a given instance.

  @param[in] copy The KSwitchKeys to copy from
  */
  KSwitchKeysCuda(const KSwitchKeysCuda &copy) = default;

  /**
  Creates a new KSwitchKeys instance by moving a given instance.

  @param[in] source The RelinKeys to move from
  */
  KSwitchKeysCuda(KSwitchKeysCuda &&source) = default;

  /**
  Copies a given KSwitchKeys instance to the current one.

  @param[in] assign The KSwitchKeys to copy from
  */
  KSwitchKeysCuda &operator=(const KSwitchKeysCuda &assign);

  /**
  Moves a given KSwitchKeys instance to the current one.

  @param[in] assign The KSwitchKeys to move from
  */
  KSwitchKeysCuda &operator=(KSwitchKeysCuda &&assign) = default;

  /**
  Returns the current number of keyswitching keys. Only keys that are
  non-empty are counted.
  */
  inline std::size_t size() const noexcept {
    return std::accumulate(keys_.cbegin(), keys_.cend(), std::size_t(0),
                           [](std::size_t res, auto &next_key) {
                             return res + (next_key.empty() ? 0 : 1);
                           });
  }

  /**
  Returns a reference to the KSwitchKeys data.
  */
  inline auto &data() noexcept { return keys_; }

  /**
  Returns a const reference to the KSwitchKeys data.
  */
  inline auto &data() const noexcept { return keys_; }

  /**
  Returns a reference to a keyswitching key at a given index.

  @param[in] index The index of the keyswitching key
  @throws std::invalid_argument if the key at the given index does not exist
  */
  inline auto &data(std::size_t index) {
    if (index >= keys_.size() || keys_[index].empty()) {
      throw std::invalid_argument("keyswitching key does not exist");
    }
    return keys_[index];
  }

  /**
  Returns a const reference to a keyswitching key at a given index.

  @param[in] index The index of the keyswitching key
  @throws std::invalid_argument if the key at the given index does not exist
  */
  inline const auto &data(std::size_t index) const {
    if (index >= keys_.size() || keys_[index].empty()) {
      throw std::invalid_argument("keyswitching key does not exist");
    }
    return keys_[index];
  }

  /**
  Returns a reference to parms_id.

  @see EncryptionParameters for more information about parms_id.
  */
  inline auto &parmsID() noexcept { return parms_id_; }

  /**
  Returns a const reference to parms_id.

  @see EncryptionParameters for more information about parms_id.
  */
  inline auto &parmsID() const noexcept { return parms_id_; }

  // /**
  // Returns an upper bound on the size of the KSwitchKeys, as if it was written
  // to an output stream.

  // @param[in] compr_mode The compression mode
  // @throws std::invalid_argument if the compression mode is not supported
  // @throws std::logic_error if the size does not fit in the return type
  // */
  // inline std::streamoff save_size(
  //     compr_mode_type compr_mode = Serialization::compr_mode_default) const
  // {
  //     std::size_t total_key_size = util::mul_safe(keys_.size(),
  //     sizeof(std::uint64_t)); // keys_dim2 for (auto &key_dim1 : keys_)
  //     {
  //         for (auto &key_dim2 : key_dim1)
  //         {
  //             total_key_size = util::add_safe(
  //                 total_key_size,
  //                 util::safe_cast<std::size_t>(key_dim2.save_size(compr_mode_type::none)));
  //         }
  //     }

  //     std::size_t members_size = Serialization::ComprSizeEstimate(
  //         util::add_safe(
  //             sizeof(parms_id_),
  //             sizeof(std::uint64_t), // keys_dim1
  //             total_key_size),
  //         compr_mode);

  //     return
  //     util::safe_cast<std::streamoff>(util::add_safe(sizeof(Serialization::SEALHeader),
  //     members_size));
  // }

  // /**
  // Saves the KSwitchKeys instance to an output stream. The output is
  // in binary format and not human-readable. The output stream must have
  // the "binary" flag set.

  // @param[out] stream The stream to save the KSwitchKeys to
  // @param[in] compr_mode The desired compression mode
  // @throws std::invalid_argument if the compression mode is not supported
  // @throws std::logic_error if the data to be saved is invalid, or if
  // compression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff save(
  //     std::ostream &stream, compr_mode_type compr_mode =
  //     Serialization::compr_mode_default) const
  // {
  //     using namespace std::placeholders;
  //     return Serialization::Save(
  //         std::bind(&KSwitchKeys::save_members, this, _1),
  //         save_size(compr_mode_type::none), stream, compr_mode, false);
  // }

  // /**
  // Loads a KSwitchKeys from an input stream overwriting the current
  // KSwitchKeys. No checking of the validity of the KSwitchKeys data against
  // encryption parameters is performed. This function should not be used unless
  // the KSwitchKeys comes from a fully trusted source.

  // @param[in] context The SEALContext
  // @param[in] stream The stream to load the KSwitchKeys from
  // @throws std::invalid_argument if the encryption parameters are not valid
  // @throws std::logic_error if the data cannot be loaded by this version of
  // Microsoft SEAL, if the loaded data is invalid, or if decompression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff unsafe_load(const SEALContext &context, std::istream
  // &stream)
  // {
  //     using namespace std::placeholders;
  //     return Serialization::Load(std::bind(&KSwitchKeys::load_members, this,
  //     context, _1, _2), stream, false);
  // }

  // /**
  // Loads a KSwitchKeys from an input stream overwriting the current
  // KSwitchKeys. The loaded KSwitchKeys is verified to be valid for the given
  // SEALContext.

  // @param[in] context The SEALContext
  // @param[in] stream The stream to load the KSwitchKeys from
  // @throws std::invalid_argument if the encryption parameters are not valid
  // @throws std::logic_error if the data cannot be loaded by this version of
  // Microsoft SEAL, if the loaded data is invalid, or if decompression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff load(const SEALContext &context, std::istream
  // &stream)
  // {
  //     KSwitchKeys new_keys;
  //     new_keys.pool_ = pool_;
  //     auto in_size = new_keys.unsafe_load(context, stream);
  //     if (!is_valid_for(new_keys, context))
  //     {
  //         throw std::logic_error("KSwitchKeys data is invalid");
  //     }
  //     std::swap(*this, new_keys);
  //     return in_size;
  // }

  // /**
  // Saves the KSwitchKeys instance to a given memory location. The output is
  // in binary format and not human-readable.

  // @param[out] out The memory location to write the KSwitchKeys to
  // @param[in] size The number of bytes available in the given memory location
  // @param[in] compr_mode The desired compression mode
  // @throws std::invalid_argument if out is null or if size is too small to
  // contain a SEALHeader, or if the compression mode is not supported
  // @throws std::logic_error if the data to be saved is invalid, or if
  // compression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff save(
  //     seal_byte *out, std::size_t size, compr_mode_type compr_mode =
  //     Serialization::compr_mode_default) const
  // {
  //     using namespace std::placeholders;
  //     return Serialization::Save(
  //         std::bind(&KSwitchKeys::save_members, this, _1),
  //         save_size(compr_mode_type::none), out, size, compr_mode, false);
  // }

  // /**
  // Loads a KSwitchKeys from a given memory location overwriting the current
  // KSwitchKeys. No checking of the validity of the KSwitchKeys data against
  // encryption parameters is performed. This function should not be used
  // unless the KSwitchKeys comes from a fully trusted source.

  // @param[in] context The SEALContext
  // @param[in] in The memory location to load the KSwitchKeys from
  // @param[in] size The number of bytes available in the given memory location
  // @throws std::invalid_argument if the encryption parameters are not valid
  // @throws std::invalid_argument if in is null or if size is too small to
  // contain a SEALHeader
  // @throws std::logic_error if the data cannot be loaded by this version of
  // Microsoft SEAL, if the loaded data is invalid, or if decompression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff unsafe_load(const SEALContext &context, const
  // seal_byte *in, std::size_t size)
  // {
  //     using namespace std::placeholders;
  //     return Serialization::Load(std::bind(&KSwitchKeys::load_members, this,
  //     context, _1, _2), in, size, false);
  // }

  // /**
  // Loads a KSwitchKeys from a given memory location overwriting the current
  // KSwitchKeys. The loaded KSwitchKeys is verified to be valid for the given
  // SEALContext.

  // @param[in] context The SEALContext
  // @param[in] in The memory location to load the KSwitchKeys from
  // @param[in] size The number of bytes available in the given memory location
  // @throws std::invalid_argument if the encryption parameters are not valid
  // @throws std::invalid_argument if in is null or if size is too small to
  // contain a SEALHeader
  // @throws std::logic_error if the data cannot be loaded by this version of
  // Microsoft SEAL, if the loaded data is invalid, or if decompression failed
  // @throws std::runtime_error if I/O operations failed
  // */
  // inline std::streamoff load(const SEALContext &context, const seal_byte *in,
  // std::size_t size)
  // {
  //     KSwitchKeys new_keys;
  //     new_keys.pool_ = pool_;
  //     auto in_size = new_keys.unsafe_load(context, in, size);
  //     if (!is_valid_for(new_keys, context))
  //     {
  //         throw std::logic_error("KSwitchKeys data is invalid");
  //     }
  //     std::swap(*this, new_keys);
  //     return in_size;
  // }

  void save(std::ostream &stream) const {
    stream.write(reinterpret_cast<const char *>(&parms_id_), sizeof(ParmsID));
    size_t n = keys_.size();
    stream.write(reinterpret_cast<char *>(&n), sizeof(size_t));
    for (size_t i = 0; i < n; i++) {
      size_t m = keys_[i].size();
      stream.write(reinterpret_cast<char *>(&m), sizeof(size_t));
      for (size_t j = 0; j < m; j++) keys_[i][j].save(stream);
    }
  }

  void load(std::istream &stream) {
    stream.read(reinterpret_cast<char *>(&parms_id_), sizeof(ParmsID));
    size_t n;
    stream.read(reinterpret_cast<char *>(&n), sizeof(size_t));
    keys_.clear();
    keys_.reserve(n);
    for (size_t i = 0; i < n; i++) {
      size_t m;
      stream.read(reinterpret_cast<char *>(&m), sizeof(size_t));
      std::vector<PublicKeyCuda> append;
      append.reserve(m);
      for (size_t j = 0; j < m; j++) {
        PublicKeyCuda p;
        p.load(stream);
        append.push_back(std::move(p));
      }
      keys_.push_back(std::move(append));
    }
  }

 private:
  // void save_members(std::ostream &stream) const;

  // void load_members(const SEALContext &context, std::istream &stream,
  // SEALVersion version);

  ParmsID parms_id_ = parmsIDZero;

  /**
  The vector of keyswitching keys.
  */
  std::vector<std::vector<PublicKeyCuda>> keys_{};
};
}  // namespace seal_gpu
