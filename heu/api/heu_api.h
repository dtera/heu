// Copyright 2024 dterazhao
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

/*!
 * Copyright (c) 2024 dterazhao.
 * \file heu_api.h
 * \author dterazhao
 * \brief C API of HEU, used for interfacing to other languages.
 */

#ifdef __cplusplus
#define HEU_EXTERN_C extern "C"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#else
#define HEU_EXTERN_C
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#endif  // __cplusplus

#include <omp.h>

#include "utils.h"

#include "heu/library/phe/encoding/encoding.h"
#include "heu/library/phe/phe.h"

#if defined(_MSC_VER) || defined(_WIN32)
#define HEU_DLL HEU_EXTERN_C __declspec(dllexport)
#else
#define HEU_DLL HEU_EXTERN_C __attribute__((visibility("default")))
#endif  // defined(_MSC_VER) || defined(_WIN32)

/**
 * @mainpage
 *
 * \brief HEU C API reference.
 *
 * For the official document page see:
 * <a href="https://www.secretflow.org.cn/docs/heu/latest/en-US">HEU</a>.
 */

#define S_Mock 0
#define S_OU 1
#define S_IPCL 2
#define S_GPaillier 2 + ENABLE_IPCL
#define S_ZPaillier 2 + ENABLE_IPCL + ENABLE_GPAILLIER
#define S_FPaillier 3 + ENABLE_IPCL + ENABLE_GPAILLIER
#define S_ClustarFPGA 4 + ENABLE_IPCL + ENABLE_GPAILLIER
#define S_ElGamal 4 + ENABLE_IPCL + ENABLE_GPAILLIER + ENABLE_CLUSTAR_FPGA
#define S_DGK 5 + ENABLE_IPCL + ENABLE_GPAILLIER + ENABLE_CLUSTAR_FPGA
#define S_DJ 6 + ENABLE_IPCL + ENABLE_GPAILLIER + ENABLE_CLUSTAR_FPGA

#define GRAIN_SIZE 256
#define SCALE 1e6
#define HE_SCALE(handle) \
  static_cast<int>(handle->GetSchemaType()) == (S_ElGamal) ? 1e4 : scale
#define BIT_OFFSET 64
#define ENABLE_BATCH_ENCODER true

#define API_BEGIN() try {
/*! \brief every function starts with API_BEGIN();
     and finishes with API_END() */
#define API_END()                                           \
  }                                                         \
  catch (std::exception const &_except_) {                  \
    std::cerr << "[Error]" << _except_.what() << std::endl; \
    return -1;                                              \
  }                                                         \
  return 0;  // NOLINT(*)

#define CHECK_NULL(handle)                                                \
  if (handle == nullptr)                                                  \
    std::cerr                                                             \
        << "HeKit/PubKey/SecKey has not been initialized or has already " \
           "been disposed.";

#define CHECK_HANDLE() CHECK_NULL(handle)

#define API(statments) \
  API_BEGIN()          \
  statments;           \
  API_END()

#define API_HANDLE(statments) API(CHECK_HANDLE() statments)

#define API_HEKIT_HANDLE(statments)                                     \
  API_HANDLE(auto *hekit = static_cast<heu::lib::phe::HeKit *>(handle); \
             statments)

#define API_DHEKIT_HANDLE(statments)                                     \
  API_HANDLE(auto *dhekit =                                              \
                 static_cast<heu::lib::phe::DestinationHeKit *>(handle); \
             statments)

#define API_HEKIT_RET(statments)                             \
  CHECK_HANDLE()                                             \
  auto *hekit = static_cast<heu::lib::phe::HeKit *>(handle); \
  statments

#define API_DHEKIT_RET(statments)                                        \
  CHECK_HANDLE()                                                         \
  auto *dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(handle); \
  statments

/*!
 * API_HEKIT_HANDLE({
 *   auto encrypter = hekit->GetEncryptor();
 *   auto encoder =
 * hekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(handle));
 *   // heu::lib::phe::Plaintext pts[len];
 *   // TIME_STAT(ParallelFor(len, n_threads,
 *   //                      [&](int i) { pts[i] = encoder.Encode(data[i]); }),
 *   //          Encode)
 * TIME_STAT(ParallelFor(len, n_threads,
 *                     [&](int i) {
 *                       // ciphers[i] = encrypter->Encrypt(pts[i]);
 *                       ciphers[i] =
 *                           encrypter->Encrypt(encoder.Encode(data[i]));
 *                     }),
 *         Encrypt)
 * });
 */
#define HE_ENCRYPTS__(HE_HANDLE, handle, get_encoder, do_encrypt)      \
  HE_HANDLE({                                                          \
    auto encrypter = handle->GetEncryptor();                           \
    get_encoder;                                                       \
    TIME_STAT(ParallelFor(len, n_threads, [&](int i) { do_encrypt; }), \
              Encrypts)                                                \
  })

#define HE_ENCRYPTS_(HE_HANDLE, handle, do_encrypt)                          \
  HE_ENCRYPTS__(                                                             \
      HE_HANDLE, handle,                                                     \
      auto encoder =                                                         \
          handle->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(handle)), \
      do_encrypt)

#define HE_ENCRYPTS(HE_HANDLE, handle) \
  HE_ENCRYPTS_(HE_HANDLE, handle,      \
               ciphers[i] = encrypter->Encrypt(encoder.Encode(data[i])))

#define HE_ENCRYPT__(HE_HANDLE, handle, get_encoder, do_encrypt) \
  HE_HANDLE({                                                    \
    auto encrypter = handle->GetEncryptor();                     \
    get_encoder;                                                 \
    do_encrypt;                                                  \
  })

#define HE_ENCRYPT(HE_HANDLE, handle)                                          \
  HE_ENCRYPT__(HE_HANDLE, handle,                                              \
               auto encoder = handle->GetEncoder<heu::lib::phe::PlainEncoder>( \
                   HE_SCALE(handle)),                                          \
               *cipher = encrypter->Encrypt(encoder.Encode(data)))

#define HE_PAIR_ENCRYPTS_(HE_HANDLE, handle, do_encrypt)                     \
  HE_ENCRYPTS__(HE_HANDLE, handle,                                           \
                heu::lib::phe::BatchEncoder encoder(handle->GetSchemaType(), \
                                                    HE_SCALE(handle)),       \
                do_encrypt)

#define HE_PAIR_ENCRYPTS(HE_HANDLE, handle) \
  HE_PAIR_ENCRYPTS_(                        \
      HE_HANDLE, handle,                    \
      ciphers[i] = encrypter->Encrypt(encoder.Encode<T>(data1[i], data2[i]));)

#define HE_PAIR_ENCRYPT(HE_HANDLE, handle)                                  \
  HE_ENCRYPT__(HE_HANDLE, handle,                                           \
               heu::lib::phe::BatchEncoder encoder(handle->GetSchemaType(), \
                                                   HE_SCALE(handle)),       \
               *cipher = encrypter->Encrypt(encoder.Encode<T>(data1, data2)))

/*!
 * API_HEKIT_HANDLE({
 * auto decrypter = hekit->GetDecryptor();
 * auto encoder =
 * hekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(handle));
 * TIME_STAT(ParallelFor(len, n_threads,
 *                       [&](int i) {
 *                         data[i] = encoder.Decode<T>(
 *                             decrypter->Decrypt(ciphers[i]));
 *                       }),
 *           Decrypt)
 * });
 */
#define HE_DECRYPTS__(get_encoder, do_decrypt)                        \
  API_HEKIT_HANDLE({                                                  \
    auto decrypter = hekit->GetDecryptor();                           \
    get_encoder;                                                      \
    TIME_STAT(ParallelFor(len, n_threads, [&](int i) { do_decrypt }), \
              Decrypts)                                               \
  });

#define HE_DECRYPTS(do_decrypt)                                                \
  HE_DECRYPTS__(auto encoder = hekit->GetEncoder<heu::lib::phe::PlainEncoder>( \
                    HE_SCALE(hekit)),                                          \
                do_decrypt);

#define HE_PAIR_DECRYPTS(do_decrypt)                                        \
  HE_DECRYPTS__(heu::lib::phe::BatchEncoder encoder(hekit->GetSchemaType(), \
                                                    HE_SCALE(hekit)),       \
                do_decrypt);

/*! \brief handle to HeKit */
typedef void *HeKitHandle;  // NOLINT(*)
/*! \brief handle to DestinationHeKit */
typedef void *DestinationHeKitHandle;  // NOLINT(*)
/*! \brief handle to PublicKey */
typedef void *PubKeyHandle;  // NOLINT(*)
/*! \brief handle to SecretKey */
typedef void *SecKeyHandle;  // NOLINT(*)

template <typename T>
inline T decode0(const heu::lib::phe::BatchEncoder &encoder,
                 const heu::lib::phe::Plaintext &pt) {
  return encoder.Decode<T, 0>(pt);
}

template <typename T>
inline T decode1(const heu::lib::phe::BatchEncoder &encoder,
                 const heu::lib::phe::Plaintext &pt) {
  return encoder.Decode<T, 1>(pt);
}

void CheckCall(int ret, const std::string &desc);

/*!
 * \brief create HeKit
 * \param schema schema type of he
 * \param key size
 * \param out the result handle of HeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int HeKitCreate(const int schema, const int key_size, HeKitHandle *out);

/*!
 * \brief create HeKit from serialized PublicKey and SecretKey
 * \param pub_key_handle handle of public key
 * \param pub_key_size the size of public key
 * \param pub_key_handle handle of secret key
 * \param pub_key_size the size of secret key
 * \param out the result handle of HeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int HeKitCreateFromKeys(const PubKeyHandle pub_key_handle,
                                const std::size_t pub_key_size,
                                const PubKeyHandle sec_key_handle,
                                const std::size_t sec_key_size,
                                HeKitHandle *out);

/*!
 * \brief free obj in handle
 * \param handle handle to be freed
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int HeKitFree(HeKitHandle handle);

/*!
 * \brief create DestinationHeKit
 * \param handle handle of public key
 * \param size the size of public key
 * \param out  the result handle of DestinationHeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int DestinationHeKitCreate(const PubKeyHandle handle,
                                   const std::size_t size,
                                   DestinationHeKitHandle *out);

/*!
 * \brief free obj in handle
 * \param handle handle to be freed
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int DestinationHeKitFree(DestinationHeKitHandle handle);

/*!
 * \brief get serialized buffer using hekit
 * \param handle HeKitHandle
 * \param out the result of serialized buffer
 * \param get_buf the function to get the serialized buffer
 * \param process_buf the function to process the serialized buffer
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int GetBuffer(
    HeKitHandle handle, void **out,
    std::function<yacl::Buffer(heu::lib::phe::HeKit *)> get_buf,
    std::function<void(yacl::Buffer &buf)> process_buf = nullptr);
/*!
 * \brief get public key using hekit
 * \param handle HeKitHandle
 * \param out the result of public key
 * \param process_buf the function to process the serialized public key
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int GetPubKey(
    HeKitHandle handle, PubKeyHandle *out,
    std::function<void(yacl::Buffer &buf)> process_buf = nullptr);

/*!
 * \brief get secret key using hekit
 * \param handle HeKitHandle
 * \param out the result of secret key
 * \param process_buf the function to process the serialized secret key
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int GetSecKey(
    HeKitHandle handle, SecKeyHandle *out,
    std::function<void(yacl::Buffer &buf)> process_buf = nullptr);

/*!
 * \brief encrypt data using hekit
 * \param handle HeKitHandle
 * \param data the data to encrypt
 * \param cipher the result of cipher
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, const T &data,
                          heu::lib::phe::Ciphertext *cipher,
                          const int64_t scale = SCALE) {
  HE_ENCRYPT(API_HEKIT_HANDLE, hekit)
}

/*!
 * \brief encrypt data using hekit
 * \param handle HeKitHandle
 * \param data1 the first data to encrypt
 * \param data2 the second data to encrypt
 * \param cipher the result of cipher
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, const T &data1, const T &data2,
                          heu::lib::phe::Ciphertext *cipher,
                          const int64_t scale = SCALE) {
#ifndef ENABLE_BATCH_ENCODER
  int128_t t1 = data1 * scale, t2 = data2 * scale;
  auto data = t1 << BIT_OFFSET | t2;
  HE_ENCRYPT(API_HEKIT_HANDLE, hekit)
#else
  HE_PAIR_ENCRYPT(API_HEKIT_HANDLE, hekit)
#endif
}

/*!
 * \brief encrypt data using hekit
 * \param handle HeKitHandle
 * \param data the data to encrypt
 * \param len the length of data
 * \param ciphers the result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, T *data, const int len,
                          heu::lib::phe::Ciphertext *ciphers,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
  HE_ENCRYPTS(API_HEKIT_HANDLE, hekit)
}

/*!
 * \brief encrypt data using hekit
 * \param handle HeKitHandle
 * \param data1 the first data to encrypt
 * \param data2 the second data to encrypt
 * \param len the length of data
 * \param ciphers the result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, T *data1, T *data2, const int len,
                          heu::lib::phe::Ciphertext *ciphers,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
#ifndef ENABLE_BATCH_ENCODER
  HE_ENCRYPTS_(API_HEKIT_HANDLE, hekit, {
    int128_t t1 = data1[i] * scale;
    int128_t t2 = data2[i] * scale;
    auto data = t1 << BIT_OFFSET | t2;
    ciphers[i] = encrypter->Encrypt(encoder.Encode(data));
  })
#else
  HE_PAIR_ENCRYPTS(API_HEKIT_HANDLE, hekit)
#endif
}

/*!
 * \brief encrypt data matrix using hekit
 * \param handle HeKitHandle
 * \param data the data matrix to encrypt
 * \param len the length of data matrix at 1st dimension
 * \param len2 the length of data matrix at 1st dimension
 * \param ciphers the result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, T **data, const int len,
                          const int len2, heu::lib::phe::Ciphertext **ciphers,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
  HE_ENCRYPTS_(API_HEKIT_HANDLE, hekit, {
    for (int j = 0; j < len2; ++j) {
      ciphers[i][j] = encrypter->Encrypt(encoder.Encode(data[i][j]));
    }
  })
}

/*!
 * \brief encrypt data matrix using hekit
 * \param handle HeKitHandle
 * \param data1 the first data matrix to encrypt
 * \param data2 the second data matrix to encrypt
 * \param len the length of data matrix at 1st dimension
 * \param len2 the length of data matrix at 1st dimension
 * \param ciphers the result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, T **data1, T **data2,
                          const int len, const int len2,
                          heu::lib::phe::Ciphertext **ciphers,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
#ifndef ENABLE_BATCH_ENCODER
  HE_ENCRYPTS_(API_HEKIT_HANDLE, hekit, {
    for (int j = 0; j < len2; ++j) {
      int128_t t1 = data1[i][j] * scale;
      int128_t t2 = data2[i][j] * scale;
      auto data = t1 << BIT_OFFSET | t2;
      ciphers[i][j] = encrypter->Encrypt(encoder.Encode(data));
    }
  })
#else
  HE_PAIR_ENCRYPTS_(API_HEKIT_HANDLE, hekit, {
    for (int j = 0; j < len2; ++j) {
      ciphers[i][j] =
          encrypter->Encrypt(encoder.Encode<T>(data1[i][j], data2[i][j]));
    }
  })
#endif
}

/*!
 * \brief encrypt data matrix using hekit
 * \param handle HeKitHandle
 * \param data the data matrix to encrypt
 * \param len the length of data matrix at 1st dimension
 * \param len2 the length of data matrix at 1st dimension
 * \param ciphers the result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Encrypt(HeKitHandle handle, T **data, const int len,
                          int *row_size, heu::lib::phe::Ciphertext **ciphers,
                          const int grain_size = GRAIN_SIZE,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
  API_HEKIT_HANDLE({
    auto encrypter = hekit->GetEncryptor();
    auto encoder =
        hekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(hekit));
    BlockedSpace2d blocked2D(
        len, [&](size_t i) { return row_size[i]; }, grain_size);
    ParallelFor2d(blocked2D, n_threads, [&](size_t i, const Range1d &r) {
      for (int j = r.begin(); j < r.end(); ++j) {
        ciphers[i][j] = encrypter->Encrypt(encoder.Encode(data[i][j]));
      }
    });
  })
}

/*!
 * \brief decrypt the ciphers using hekit
 * \param handle HeKitHandle
 * \param cipher cipher to decrypt
 * \param data the result of ciphers
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          const heu::lib::phe::Ciphertext &cipher, T *data,
                          const int64_t scale = SCALE) {
  API_HEKIT_HANDLE({
    auto decrypter = hekit->GetDecryptor();
    auto encoder =
        hekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(hekit));
    *data = encoder.Decode<T>(decrypter->Decrypt(cipher));
  })
}

/*!
 * \brief decrypt the ciphers using hekit
 * \param handle HeKitHandle
 * \param cipher cipher to decrypt
 * \param data1 the first result of ciphers
 * \param data2 the second result of ciphers
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          const heu::lib::phe::Ciphertext &cipher, T *data1,
                          T *data2, const int64_t scale = SCALE) {
#ifndef ENABLE_BATCH_ENCODER
  API_HEKIT_HANDLE({
    auto decrypter = hekit->GetDecryptor();
    auto pt = decrypter->Decrypt(cipher);
    auto scale1 = HE_SCALE(hekit);
    auto encoder = hekit->GetEncoder<heu::lib::phe::PlainEncoder>(scale1);
    auto data = encoder.Decode<int128_t>(pt);
    *data1 = static_cast<T>(data >> 64) / scale1;
    *data2 = static_cast<T>(static_cast<int64_t>(data)) / scale1;
  })
#else
  API_HEKIT_HANDLE({
    auto decrypter = hekit->GetDecryptor();
    auto pt = decrypter->Decrypt(cipher);
    heu::lib::phe::BatchEncoder encoder(hekit->GetSchemaType(),
                                        HE_SCALE(hekit));
    *data1 = decode0<T>(encoder, pt);
    *data2 = decode1<T>(encoder, pt);
  })
#endif
}

/*!
 * \brief decrypt the ciphers using hekit
 * \param handle HeKitHandle
 * \param ciphers ciphers to decrypt
 * \param len the length of data
 * \param data the result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          heu::lib::phe::Ciphertext *ciphers, const int len,
                          T *data, const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
  HE_DECRYPTS(data[i] = encoder.Decode<T>(decrypter->Decrypt(ciphers[i]));)
}

/*!
 * \brief decrypt the ciphers using hekit
 * \param handle HeKitHandle
 * \param ciphers ciphers to decrypt
 * \param len the length of data
 * \param data1 the first result of ciphers
 * \param data2 the second result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          heu::lib::phe::Ciphertext *ciphers, const int len,
                          T *data1, T *data2, const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
#ifndef ENABLE_BATCH_ENCODER
  HE_DECRYPTS({
    auto data = encoder.Decode<int128_t>(decrypter->Decrypt(ciphers[i]));
    data1[i] = (static_cast<T>(data >> 64)) / scale;
    data2[i] = static_cast<T>(static_cast<int64_t>(data)) / scale;
  })
#else
  HE_PAIR_DECRYPTS({
    auto pt = decrypter->Decrypt(ciphers[i]);
    data1[i] = decode0<T>(encoder, pt);
    data2[i] = decode1<T>(encoder, pt);
  })
#endif
}

/*!
 * \brief decrypt the ciphers matrix using hekit
 * \param handle HeKitHandle
 * \param ciphers ciphers matrix to decrypt
 * \param len the length of data matrix at 1st dimension
 * \param len the length of data matrix at 2st dimension
 * \param data the result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          heu::lib::phe::Ciphertext **ciphers, const int len,
                          const int len2, T **data, const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
  HE_DECRYPTS({
    for (int j = 0; j < len2; ++j) {
      data[i][j] = encoder.Decode<T>(decrypter->Decrypt(ciphers[i][j]));
    }
  })
}

/*!
 * \brief decrypt the ciphers matrix using hekit
 * \param handle HeKitHandle
 * \param ciphers ciphers matrix to decrypt
 * \param len the length of data matrix at 1st dimension
 * \param len the length of data matrix at 2st dimension
 * \param data1 the first result of ciphers matrix
 * \param data2 the second result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int Decrypt(HeKitHandle handle,
                          heu::lib::phe::Ciphertext **ciphers, const int len,
                          const int len2, T **data1, T **data2,
                          const int64_t scale = SCALE,
                          const int32_t n_threads = omp_get_num_procs()) {
#ifndef ENABLE_BATCH_ENCODER
  HE_DECRYPTS({
    for (int j = 0; j < len2; ++j) {
      auto data = encoder.Decode<int128_t>(decrypter->Decrypt(ciphers[i][j]));
      data1[i][j] = static_cast<T>(data >> 64) / scale;
      data2[i][j] = static_cast<T>(static_cast<int64_t>(data)) / scale;
    }
  })
#else
  HE_PAIR_DECRYPTS({
    for (int j = 0; j < len2; ++j) {
      auto pt = decrypter->Decrypt(ciphers[i][j]);
      data1[i][j] = decode0<T>(encoder, pt);
      data2[i][j] = decode1<T>(encoder, pt);
    }
  })
#endif
}

/*!
 * \brief encrypt data using DestinationHekit
 * \param handle HeKitHandle
 * \param data the data to encrypt
 * \param cipher the result of cipher
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int DestinationHeKitEncrypt(DestinationHeKitHandle handle,
                                          const T &data,
                                          heu::lib::phe::Ciphertext *cipher,
                                          const int64_t scale = SCALE) {
  HE_ENCRYPT(API_DHEKIT_HANDLE, dhekit)
}

/*!
 * \brief encrypt data using DestinationHekit
 * \param handle HeKitHandle
 * \param data1 the first data to encrypt
 * \param data2 the second data to encrypt
 * \param cipher the result of cipher
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int DestinationHeKitEncrypt(DestinationHeKitHandle handle,
                                          const T &data1, const T &data2,
                                          heu::lib::phe::Ciphertext *cipher,
                                          const int64_t scale = SCALE) {
#ifndef ENABLE_BATCH_ENCODER
  int128_t t1 = data1 * scale, t2 = data2 * scale;
  auto data = t1 << BIT_OFFSET | t2;
  HE_ENCRYPT(API_DHEKIT_HANDLE, dhekit)
#else
  HE_PAIR_ENCRYPT(API_DHEKIT_HANDLE, dhekit)
#endif
}

/*!
 * \brief encrypt data using DestinationHekit
 * \param handle HeKitHandle
 * \param data the data to encrypt
 * \param len the length of data
 * \param ciphers the result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int DestinationHeKitEncrypt(
    DestinationHeKitHandle handle, T *data, const int len,
    heu::lib::phe::Ciphertext *ciphers, const int64_t scale = SCALE,
    const int32_t n_threads = omp_get_num_procs()) {
  HE_ENCRYPTS(API_DHEKIT_HANDLE, dhekit)
}

/*!
 * \brief encrypt data using DestinationHekit
 * \param handle HeKitHandle
 * \param data1 the first data to encrypt
 * \param data2 the second data to encrypt
 * \param len the length of data
 * \param ciphers the result of ciphers
 * \param scale scale of the data
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int DestinationHeKitEncrypt(
    DestinationHeKitHandle handle, T *data1, T *data2, const int len,
    heu::lib::phe::Ciphertext *ciphers, const int64_t scale = SCALE,
    const int32_t n_threads = omp_get_num_procs()) {
#ifndef ENABLE_BATCH_ENCODER
  HE_ENCRYPTS_(API_DHEKIT_HANDLE, dhekit, {
    int128_t t1 = data1[i] * scale;
    int128_t t2 = data2[i] * scale;
    auto data = t1 << BIT_OFFSET | t2;
    ciphers[i] = encrypter->Encrypt(encoder.Encode(data));
  })
#else
  HE_PAIR_ENCRYPTS(API_DHEKIT_HANDLE, dhekit)
#endif
}

/*!
 * \brief encrypt data matrix using DestinationHekit
 * \param handle HeKitHandle
 * \param data the data matrix to encrypt
 * \param len the length of data matrix at 1st dimension
 * \param len2 the length of data matrix at 1st dimension
 * \param ciphers the result of ciphers matrix
 * \param scale scale of the data matrix
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int DestinationHeKitEncrypt(
    HeKitHandle handle, T **data, const int len, int *row_size,
    heu::lib::phe::Ciphertext **ciphers, const int grain_size = GRAIN_SIZE,
    const int64_t scale = SCALE,
    const int32_t n_threads = omp_get_num_procs()) {
  API_DHEKIT_HANDLE({
    auto encrypter = dhekit->GetEncryptor();
    auto encoder =
        dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(dhekit));
    BlockedSpace2d blocked2D(
        len, [&](size_t i) { return row_size[i]; }, grain_size);
    ParallelFor2d(blocked2D, n_threads, [&](size_t i, const Range1d &r) {
      for (int j = r.begin(); j < r.end(); ++j) {
        ciphers[i][j] = encrypter->Encrypt(encoder.Encode(data[i][j]));
      }
    });
  })
}

/*!
 * \brief init ciphers using DestinationHekit
 * \param handle HeKitHandle
 * \param data the data to init
 * \param ciphers the result of ciphers
 * \param len the length of ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
void DHeKitCiphersInit(DestinationHeKitHandle handle,
                       [[maybe_unused]] const T &data,
                       heu::lib::phe::Ciphertext *ciphers, const int len) {
  CheckCall(DestinationHeKitEncrypt(handle, 0, ciphers, 1), "DHeKitEncrypt");
  auto buf = ciphers[0].Serialize();
  for (int i = 0; i < len; ++i) {
    ciphers[i].Deserialize(buf);
  }
}

/*!
 * \brief add the cipher inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be added, also as a result
 * \param c2 second cipher to be added
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddCipherInplace(DestinationHeKitHandle handle,
                             heu::lib::phe::Ciphertext &c1,
                             const heu::lib::phe::Ciphertext &c2);

/*!
 * \brief subtract the cipher inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be subtracted, also as a result
 * \param c2 second cipher to be subtracted
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubCipherInplace(DestinationHeKitHandle handle,
                             heu::lib::phe::Ciphertext &c1,
                             const heu::lib::phe::Ciphertext &c2);

/*!
 * \brief add the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be added, also as a result
 * \param cs2 the ciphers to be added
 * \param len the length of the ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddCiphersInplace(DestinationHeKitHandle handle,
                              heu::lib::phe::Ciphertext *cs1,
                              const heu::lib::phe::Ciphertext *cs2,
                              const int len,
                              const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief subtract the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be subtracted, also as a result
 * \param cs2 the ciphers to be subtracted
 * \param len the length of the ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubCiphersInplace(DestinationHeKitHandle handle,
                              heu::lib::phe::Ciphertext *cs1,
                              const heu::lib::phe::Ciphertext *cs2,
                              const int len,
                              const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief scatter add the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param results the ciphers to be subtracted, also as a result
 * \param operands the ciphers to be subtracted
 * \param indexes the indexes of the ciphers to be subtracted
 * \param len the length of the results
 * \param len2 the length of the operands
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int ScatterAddCiphersInplace(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *results,
    const heu::lib::phe::Ciphertext *operands, const long *indexes,
    [[maybe_unused]] const int len, const int len2, bool parallel = true,
    const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief scatter subtract the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be subtracted, also as a result
 * \param cs2 the ciphers to be subtracted
 * \param indexes the indexes of the ciphers to be subtracted
 * \param len the length of the ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int ScatterSubCiphersInplace(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *results,
    const heu::lib::phe::Ciphertext *operands, const long *indexes,
    [[maybe_unused]] const int len, const int len2, bool parallel = false,
    const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief add the ciphers inplace at 1st dimension using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be added, also as a result
 * \param cs2 the ciphers to be added
 * \param len1 the length of the ciphers at 1st dimension
 * \param len2 the length of the ciphers at 2st dimension
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddCiphersInplaceAxis0(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *cs1,
    heu::lib::phe::Ciphertext **cs2, const int len1, const int len2,
    const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief add the ciphers inplace at 1st dimension using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be added, also as a result
 * \param cs2 the ciphers to be added
 * \param row_size the length of the ciphers at 2st dimension
 * \param len the length of the ciphers at 1st dimension
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddCiphersInplaceAxis0_(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *cs1,
    heu::lib::phe::Ciphertext **cs2, int *row_size, const int len,
    const int grain_size = GRAIN_SIZE,
    [[maybe_unused]] const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief subtract the ciphers inplace at 1st dimension using
 * DestinationHeKit \param handle DestinationHeKitHandle \param cs1 the
 * ciphers to be subtracted, also as a result \param cs2 the ciphers to be
 * subtracted \param len1 the length of the ciphers at 1st dimension \param
 * len2 the length of the ciphers at 2st dimension \param n_threads the
 * number of thread \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubCiphersInplaceAxis0(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *cs1,
    heu::lib::phe::Ciphertext **cs2, const int len1, const int len2,
    const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief subtract the ciphers inplace at 1st dimension using
 * DestinationHeKit \param handle DestinationHeKitHandle \param cs1 the
 * ciphers to be subtracted, also as a result \param cs2 the ciphers to be
 * subtracted \param row_size the length of the ciphers at 2st dimension
 * \param len the length of the ciphers at 1st dimension
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubCiphersInplaceAxis0_(
    DestinationHeKitHandle handle, heu::lib::phe::Ciphertext *cs1,
    heu::lib::phe::Ciphertext **cs2, int *row_size, const int len,
    const int grain_size,
    [[maybe_unused]] const int32_t n_threads = omp_get_num_procs());

/*!
 * \brief add the cipher and the plaintext inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be added, also as a result
 * \param p the plaintext to be added
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddPlainInplace(DestinationHeKitHandle handle,
                            heu::lib::phe::Ciphertext &c,
                            const heu::lib::phe::Plaintext &p);

/*!
 * \brief subtract the cipher and the plaintext inplace using
 * DestinationHeKit \param handle DestinationHeKitHandle \param c the cipher
 * to be subtracted, also as a result \param p the plaintext to be
 * subtracted \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubPlainInplace(DestinationHeKitHandle handle,
                            heu::lib::phe::Ciphertext &c,
                            const heu::lib::phe::Plaintext &p);

/*!
 * \brief multiply the cipher and the plaintext inplace using
 * DestinationHeKit \param handle DestinationHeKitHandle \param c the cipher
 * to be multiplied, also as a result \param p the plaintext to be
 * multiplied \return 0 when success, -1 when failure happens
 */
HEU_DLL int MultiPlainInplace(DestinationHeKitHandle handle,
                              heu::lib::phe::Ciphertext &c,
                              const heu::lib::phe::Plaintext &p);

/*!
 * \brief add the cipher using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be added
 * \param c2 the second cipher to be added
 * \return the added result between the two ciphers above
 */
inline heu::lib::phe::Ciphertext AddCipher(
    DestinationHeKitHandle handle, const heu::lib::phe::Ciphertext &c1,
    const heu::lib::phe::Ciphertext &c2) {
  API_DHEKIT_RET({ return dhekit->GetEvaluator()->Add(c1, c2); })
}

/*!
 * \brief subtract the cipher using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be subtracted
 * \param c2 the second cipher to be subtracted
 * \return the subtracted result between the two ciphers above
 */
inline heu::lib::phe::Ciphertext SubCipher(
    DestinationHeKitHandle handle, const heu::lib::phe::Ciphertext &c1,
    const heu::lib::phe::Ciphertext &c2) {
  API_DHEKIT_RET({ return dhekit->GetEvaluator()->Sub(c1, c2); })
}

/*!
 * \brief add the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be added
 * \param p the plaintext to be added
 * \return the added result between the cipher and the plaintext above
 */
inline heu::lib::phe::Ciphertext AddPlain(DestinationHeKitHandle handle,
                                          const heu::lib::phe::Ciphertext &c,
                                          const heu::lib::phe::Plaintext &p) {
  API_DHEKIT_RET({ return dhekit->GetEvaluator()->Add(c, p); })
}

/*!
 * \brief subtract the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be subtracted
 * \param p the plaintext to be subtracted
 * \return the added result between the cipher and the plaintext above
 */
inline heu::lib::phe::Ciphertext SubPlain(DestinationHeKitHandle handle,
                                          const heu::lib::phe::Ciphertext &c,
                                          const heu::lib::phe::Plaintext &p) {
  API_DHEKIT_RET({ return dhekit->GetEvaluator()->Sub(c, p); })
}

/*!
 * \brief multiply the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be multiplied
 * \param p the plaintext to be multiplied
 * \return the multiplied result between the cipher and the plaintext above
 */
inline heu::lib::phe::Ciphertext MultiPlain(DestinationHeKitHandle handle,
                                            const heu::lib::phe::Ciphertext &c,
                                            const heu::lib::phe::Plaintext &p) {
  API_DHEKIT_RET({ return dhekit->GetEvaluator()->Mul(c, p); })
}

/*!
 * \brief add the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be added
 * \param p the plaintext to be added
 * \param scale scale of the data
 * \return the added result between the cipher and the plaintext above
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] inline heu::lib::phe::Ciphertext AddPlain(
    DestinationHeKitHandle handle, const heu::lib::phe::Ciphertext &c,
    const T &p, const int64_t scale = SCALE) {
  API_DHEKIT_RET({
    return dhekit->GetEvaluator()->Add(
        c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(dhekit))
               .Encode(p));
  })
}

/*!
 * \brief subtract the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be subtracted
 * \param scale scale of the data
 * \param p the plaintext to be subtracted
 * \return the added result between the cipher and the plaintext above
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] inline heu::lib::phe::Ciphertext SubPlain(
    DestinationHeKitHandle handle, const heu::lib::phe::Ciphertext &c,
    const T &p, const int64_t scale = SCALE) {
  API_DHEKIT_RET({
    return dhekit->GetEvaluator()->Sub(
        c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(dhekit))
               .Encode(p));
  })
}

/*!
 * \brief multiply the cipher and the plaintext using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be multiplied
 * \param p the plaintext to be multiplied
 * \param scale scale of the data
 * \return the multiplied result between the cipher and the plaintext above
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] inline heu::lib::phe::Ciphertext MultiPlain(
    DestinationHeKitHandle handle, const heu::lib::phe::Ciphertext &c,
    const T &p, const int64_t scale = 1) {
  API_DHEKIT_RET({
    return dhekit->GetEvaluator()->Mul(
        c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(scale).Encode(p));
  })
}

/*!
 * \brief add the cipher and the plaintext inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be added, also as a result
 * \param p the plaintext to be added
 * \param scale scale of the data
 * \return 0 when success, -1 when failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int AddPlainInplace(DestinationHeKitHandle handle,
                                  heu::lib::phe::Ciphertext &c, const T &p,
                                  const int64_t scale = SCALE) {
  API_DHEKIT_HANDLE({
    dhekit->GetEvaluator()->AddInplace(
        &c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(dhekit))
                .Encode(p));
  })
}

/*!
 * \brief subtract the cipher and the plaintext inplace using
 * DestinationHeKit \param handle DestinationHeKitHandle \param c the cipher
 * to be subtracted, also as a result \param p the plaintext to be
 * subtracted \param scale scale of the data \return 0 when success, -1 when
 * failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int SubPlainInplace(DestinationHeKitHandle handle,
                                  heu::lib::phe::Ciphertext &c, const T &p,
                                  const int64_t scale = SCALE) {
  API_DHEKIT_HANDLE({
    dhekit->GetEvaluator()->SubInplace(
        &c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(HE_SCALE(dhekit))
                .Encode(p));
  })
}

/*!
 * \brief multiply the cipher and the plaintext inplace using
 * DestinationHeKit \param handle DestinationHeKitHandle \param c the cipher
 * to be multiplied, also as a result \param p the plaintext to be
 * multiplied \param scale scale of the data \return 0 when success, -1 when
 * failure happens
 */
template <typename T,
          typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
[[nodiscard]] int MultiPlainInplace(DestinationHeKitHandle handle,
                                    heu::lib::phe::Ciphertext &c, const T &p,
                                    const int64_t scale = 1){API_DHEKIT_HANDLE({
  dhekit->GetEvaluator()->MulInplace(
      &c, dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(scale).Encode(p));
})}

/*!
 * \brief sum the ciphers using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs the ciphers to sum
 * \param len the length of the ciphers
 * \param out the cipher as a result
 * \param min_work_size min size of parallelism
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int SumCiphers(DestinationHeKitHandle handle,
                   heu::lib::phe::Ciphertext *ciphers, const int len,
                   heu::lib::phe::Ciphertext *out, int32_t min_work_size = 1000,
                   const int32_t n_threads = omp_get_num_procs());
