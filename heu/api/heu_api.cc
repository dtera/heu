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

#include "heu/api/heu_api.h"

void CheckCall(int ret, const std::string &desc) {
  if (ret != 0) {
    std::cerr << "Failed to call api about heu: " << desc << std::endl;
  }
}

/*!
 * \brief create HeKit
 * \param schema schema type of he
 * \param key size
 * \param out the result handle of HeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int HeKitCreate(const int schema, const int key_size,
                        HeKitHandle *out){API({
  *out = new heu::lib::phe::HeKit(
      static_cast<heu::lib::phe::SchemaType>(schema), key_size);
})}

/*!
 * \brief create HeKit from serialized PublicKey and SecretKey
 * \param pub_key_handle handle of public key
 * \param pub_key_size the size of public key
 * \param pub_key_handle handle of secret key
 * \param pub_key_size the size of secret key
 * \param out the result handle of HeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int HeKitCreateFromKeys(const PubKeyHandle pub_key_handle,
                            const std::size_t pub_key_size,
                            const PubKeyHandle sec_key_handle,
                            const std::size_t sec_key_size, HeKitHandle *out){
        CHECK_NULL(pub_key_handle) CHECK_NULL(sec_key_handle) API({
          yacl::Buffer pk_buffer(pub_key_handle, pub_key_size);
          yacl::Buffer sk_buffer(sec_key_handle, sec_key_size);
          *out = new heu::lib::phe::HeKit(pk_buffer, sk_buffer);
        })}

/*!
 * \brief free obj in handle
 * \param handle handle to be freed
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int HeKitFree(HeKitHandle handle){API_HEKIT_HANDLE(delete hekit)}

/*!
 * \brief create DestinationHeKit
 * \param handle handle of public key
 * \param size the size of public key
 * \param out the result handle of DestinationHeKit
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int DestinationHeKitCreate(const PubKeyHandle handle,
                                   const std::size_t size,
                                   DestinationHeKitHandle *out){API_HANDLE({
  yacl::Buffer pk_buffer(handle, size);
  *out = new heu::lib::phe::DestinationHeKit(pk_buffer);
})}

/*!
 * \brief free obj in handle
 * \param handle handle to be freed
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int DestinationHeKitFree(DestinationHeKitHandle handle){
    API_DHEKIT_HANDLE(delete dhekit)}

/*!
 * \brief get serialized buffer using hekit
 * \param handle HeKitHandle
 * \param out the result of serialized buffer
 * \param get_buf the function to get the serialized buffer
 * \param process_buf the function to process the serialized buffer
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int GetBuffer(HeKitHandle handle, void **out,
                  std::function<yacl::Buffer(heu::lib::phe::HeKit *)> get_buf,
                  std::function<void(yacl::Buffer &buf)> process_buf){
        API_HEKIT_HANDLE({
          auto buf = get_buf(hekit);
          if (buf.size() > 0) {
            if (process_buf == nullptr) {
              *out = malloc(buf.size());
              std::memcpy(*out, buf.data(), buf.size());
            } else {
              process_buf(buf);
            }
          }
        })}

/*!
 * \brief get public key using hekit
 * \param handle HeKitHandle
 * \param out the result of public key
 * \param process_buf the function to process the serialized public key
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int GetPubKey(HeKitHandle handle, PubKeyHandle *out,
                      std::function<void(yacl::Buffer &buf)> process_buf) {
  return GetBuffer(
      handle, out,
      [&](auto hekit) { return std::move(hekit->GetPublicKey()->Serialize()); },
      process_buf);
}

/*!
 * \brief get secret key using hekit
 * \param handle HeKitHandle
 * \param out the result of secret key
 * \param process_buf the function to process the serialized secret key
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int GetSecKey(HeKitHandle handle, SecKeyHandle *out,
                      std::function<void(yacl::Buffer &buf)> process_buf) {
  return GetBuffer(
      handle, out,
      [&](auto hekit) { return std::move(hekit->GetSecretKey()->Serialize()); },
      process_buf);
}

/*!
 * \brief add the cipher inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be added, also as the result
 * \param c2 second cipher to be added
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddCipherInplace(DestinationHeKitHandle handle,
                             heu::lib::phe::Ciphertext &c1,
                             const heu::lib::phe::Ciphertext &c2){
    API_DHEKIT_HANDLE({ dhekit->GetEvaluator()->AddInplace(&c1, c2); })}

/*!
 * \brief substract the cipher inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c1 the first cipher to be substracted, also as the result
 * \param c2 second cipher to be substracted
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubCipherInplace(DestinationHeKitHandle handle,
                             heu::lib::phe::Ciphertext &c1,
                             const heu::lib::phe::Ciphertext &c2){
    API_DHEKIT_HANDLE({ dhekit->GetEvaluator()->SubInplace(&c1, c2); })}

/*!
 * \brief add the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be added, also as a result
 * \param cs2 the ciphers to be added
 * \param len the length of the ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int AddCiphersInplace(DestinationHeKitHandle handle,
                          heu::lib::phe::Ciphertext *cs1,
                          const heu::lib::phe::Ciphertext *cs2, const int len,
                          const int32_t n_threads){API_DHEKIT_HANDLE({
      ParallelFor(len, n_threads, [&](int i) {
        dhekit->GetEvaluator()->AddInplace(&cs1[i], cs2[i]);
      });
    })}

/*!
 * \brief subtract the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be subtracted, also as a result
 * \param cs2 the ciphers to be subtracted
 * \param len the length of the ciphers
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int SubCiphersInplace(DestinationHeKitHandle handle,
                          heu::lib::phe::Ciphertext *cs1,
                          const heu::lib::phe::Ciphertext *cs2, const int len,
                          const int32_t n_threads){API_DHEKIT_HANDLE({
      ParallelFor(len, n_threads, [&](int i) {
        dhekit->GetEvaluator()->SubInplace(&cs1[i], cs2[i]);
      });
    })}

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
HEU_DLL
    int ScatterAddCiphersInplace(DestinationHeKitHandle handle,
                                 heu::lib::phe::Ciphertext *results,
                                 const heu::lib::phe::Ciphertext *operands,
                                 const long *indexes,
                                 [[maybe_unused]] const int len, const int len2,
                                 bool parallel,
                                 const int32_t n_threads){API_DHEKIT_HANDLE({
      if (parallel) {
        /*omp_lock_t *write_locks = new omp_lock_t[len];
        for (size_t i = 0; i < len; i++) {
          omp_init_lock(&(write_locks[i]));
        }*/
        ParallelFor(len2, n_threads, [&](int i) {
          // omp_set_lock(&(write_locks[indexes[i]]));
          dhekit->GetEvaluator()->AddInplace(&results[indexes[i]], operands[i]);
          // omp_unset_lock(&(write_locks[indexes[i]]));
        });
        /*for (size_t i = 0; i < len; i++) {
          omp_destroy_lock(&(write_locks[i]));
        }
        delete[] (write_locks);*/
      } else {
        /*for (int i = 0; i < len2; ++i) {
          dhekit->GetEvaluator()->AddInplace(&results[indexes[i]], operands[i]);
        }*/
        ParallelFor(len2, [&](int i) {
          dhekit->GetEvaluator()->AddInplace(&results[indexes[i]], operands[i]);
        });
      }
    })}

/*!
 * \brief scatter subtract the ciphers inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param results the ciphers to be subtracted, also as a result
 * \param operands the ciphers to be subtracted
 * \param indexes the indexes of the ciphers to be subtracted
 * \param len the length of the results
 * \param len2 the length of the operands
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int ScatterSubCiphersInplace(DestinationHeKitHandle handle,
                                 heu::lib::phe::Ciphertext *results,
                                 const heu::lib::phe::Ciphertext *operands,
                                 const long *indexes,
                                 [[maybe_unused]] const int len, const int len2,
                                 bool parallel,
                                 const int32_t n_threads){API_DHEKIT_HANDLE({
      if (parallel) {
        /*omp_lock_t *write_locks = new omp_lock_t[len];
        for (size_t i = 0; i < len; i++) {
          omp_init_lock(&(write_locks[i]));
        }*/
        ParallelFor(len2, n_threads, [&](int i) {
          // omp_set_lock(&(write_locks[indexes[i]]));
          dhekit->GetEvaluator()->SubInplace(&results[indexes[i]], operands[i]);
          // omp_unset_lock(&(write_locks[indexes[i]]));
        });
        /*for (size_t i = 0; i < len; i++) {
          omp_destroy_lock(&(write_locks[i]));
        }
        delete[] (write_locks);*/
      } else {
        /*for (int i = 0; i < len; ++i) {
          dhekit->GetEvaluator()->SubInplace(&results[indexes[i]], operands[i]);
        }*/
        ParallelFor(len2, [&](int i) {
          dhekit->GetEvaluator()->AddInplace(&results[indexes[i]], operands[i]);
        });
      }
    })}

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
HEU_DLL
    int AddCiphersInplaceAxis0(DestinationHeKitHandle handle,
                               heu::lib::phe::Ciphertext *cs1,
                               heu::lib::phe::Ciphertext **cs2, const int len1,
                               const int len2, const int32_t n_threads){
        // handle to add ciphers at axis 0
        API_DHEKIT_HANDLE({
          auto evaluator = dhekit->GetEvaluator();
          ParallelFor(len1, n_threads, [&](int i) {
            for (int j = 0; j < len2; ++j) {
              evaluator->AddInplace(&cs1[i], cs2[i][j]);
            }
          });
        })}

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
HEU_DLL
    int AddCiphersInplaceAxis0_(DestinationHeKitHandle handle,
                                heu::lib::phe::Ciphertext *cs1,
                                heu::lib::phe::Ciphertext **cs2, int *row_size,
                                const int len, const int grain_size,
                                [[maybe_unused]] const int32_t n_threads){
        // handle to add ciphers at axis 0
        API_DHEKIT_HANDLE({
          auto evaluator = dhekit->GetEvaluator();
          ParallelFor(len, n_threads, [&](int i) {
            for (int j = 0; j < row_size[i]; ++j) {
              evaluator->AddInplace(&cs1[i], cs2[i][j]);
            }
          });
          /*BlockedSpace2d blocked2D(
              len, [&](size_t i) { return row_size[i]; }, grain_size);
          ParallelFor2d(blocked2D, n_threads, [&](size_t i, const Range1d &r) {
            for (int j = r.begin(); j < r.end(); ++j) {
              evaluator->AddInplace(&cs1[i], cs2[i][j]);
            }
          });*/
        })}

/*!
 * \brief subtract the ciphers inplace at 1st dimension using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be subtracted, also as a result
 * \param cs2 the ciphers to be subtracted
 * \param len1 the length of the ciphers at 1st dimension
 * \param len2 the length of the ciphers at 2st dimension
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int SubCiphersInplaceAxis0(DestinationHeKitHandle handle,
                               heu::lib::phe::Ciphertext *cs1,
                               heu::lib::phe::Ciphertext **cs2, const int len1,
                               const int len2, const int32_t n_threads){
        // handle to sub ciphers at axis 0
        API_DHEKIT_HANDLE({
          auto evaluator = dhekit->GetEvaluator();
          ParallelFor(len1, n_threads, [&](int i) {
            for (int j = 0; j < len2; ++j) {
              evaluator->SubInplace(&cs1[i], cs2[i][j]);
            }
          });
        })}

/*!
 * \brief subtract the ciphers inplace at 1st dimension using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param cs1 the ciphers to be subtracted, also as a result
 * \param cs2 the ciphers to be subtracted
 * \param row_size the length of the ciphers at 2st dimension
 * \param len the length of the ciphers at 1st dimension
 * \param n_threads the number of thread
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL
    int SubCiphersInplaceAxis0_(DestinationHeKitHandle handle,
                                heu::lib::phe::Ciphertext *cs1,
                                heu::lib::phe::Ciphertext **cs2, int *row_size,
                                const int len, const int grain_size,
                                [[maybe_unused]] const int32_t n_threads){
        // handle to sub ciphers at axis 0
        API_DHEKIT_HANDLE({
          auto evaluator = dhekit->GetEvaluator();
          ParallelFor(len, n_threads, [&](int i) {
            for (int j = 0; j < row_size[i]; ++j) {
              evaluator->SubInplace(&cs1[i], cs2[i][j]);
            };
          });
          /*BlockedSpace2d blocked2D(
              len, [&](size_t i) { return row_size[i]; }, grain_size);
          ParallelFor2d(blocked2D, n_threads, [&](size_t i, const Range1d &r) {
            for (int j = r.begin(); j < r.end(); ++j) {
              evaluator->SubInplace(&cs1[i], cs2[i][j]);
            }
          });*/
        })}

/*!
 * \brief add the cipher and the pliantext inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be added, also as the result
 * \param p the plaintext to be added
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int AddPlainInplace(DestinationHeKitHandle handle,
                            heu::lib::phe::Ciphertext &c,
                            const heu::lib::phe::Plaintext &p){
    API_DHEKIT_HANDLE({ dhekit->GetEvaluator()->AddInplace(&c, p); })}

/*!
 * \brief substract the cipher and the pliantext inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be substracted, also as the result
 * \param p the plaintext to be substracted
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int SubPlainInplace(DestinationHeKitHandle handle,
                            heu::lib::phe::Ciphertext &c,
                            const heu::lib::phe::Plaintext &p){
    API_DHEKIT_HANDLE({ dhekit->GetEvaluator()->SubInplace(&c, p); })}

/*!
 * \brief multiply the cipher and the pliantext inplace using DestinationHeKit
 * \param handle DestinationHeKitHandle
 * \param c the cipher to be multiplied, also as the result
 * \param p the plaintext to be multiplied
 * \return 0 when success, -1 when failure happens
 */
HEU_DLL int MultiPlainInplace(DestinationHeKitHandle handle,
                              heu::lib::phe::Ciphertext &c,
                              const heu::lib::phe::Plaintext &p){
    API_DHEKIT_HANDLE({ dhekit->GetEvaluator()->MulInplace(&c, p); })}

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
HEU_DLL int SumCiphers(DestinationHeKitHandle handle,
                       heu::lib::phe::Ciphertext *ciphers, const int len,
                       heu::lib::phe::Ciphertext *out, int32_t min_work_size,
                       const int32_t n_threads) {
  CheckCall(DestinationHeKitEncrypt(handle, 0, out, 1), "DHeKitEncrypt");
  API_DHEKIT_HANDLE({
    auto evaluator = dhekit->GetEvaluator();
    auto add_op = [&](int size, heu::lib::phe::Ciphertext *cs) {
      for (int i = 0; i < size; ++i) {
        evaluator->AddInplace(out, cs[i]);
      };
    };
    if (len < min_work_size) {
      add_op(len, ciphers);
    } else {
      heu::lib::phe::Ciphertext cs[n_threads];
      DHeKitCiphersInit(handle, 0, cs, n_threads);
      ParallelFor(len, n_threads, [&](int i) {
        auto tidx = omp_get_thread_num();
        evaluator->AddInplace(&cs[tidx], ciphers[i]);
      });
      add_op(n_threads, cs);
    };
  })
}
