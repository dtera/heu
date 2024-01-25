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

#include <gtest/gtest.h>

#include "heu/api/heu_api.h"

TEST(HEU, API_OU_HeKit) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);
  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);

  HeKitHandle handle2;
  std::size_t pub_key_size = hekit->GetPublicKey()->Serialize().size();
  std::size_t sec_key_size = hekit->GetSecretKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  SecKeyHandle skhandle;
  GetSecKey(hekit, &skhandle);
  HeKitCreateFromKeys(pkhandle, pub_key_size, skhandle, sec_key_size, &handle2);
  auto hekit2 = static_cast<heu::lib::phe::HeKit *>(handle2);

  auto encoder = hekit->GetEncoder<heu::lib::phe::PlainEncoder>(1e7);
  auto encryptor = hekit->GetEncryptor();
  auto evaluator = hekit2->GetEvaluator();
  auto decryptor = hekit->GetDecryptor();
  auto ct1 = encryptor->Encrypt(encoder.Encode(1122.2222221));
  auto ct2 = encryptor->Encrypt(encoder.Encode(1133.3333331));
  auto ct3 = evaluator->Add(ct1, ct2);
  auto res = decryptor->Decrypt(ct3);
  std::cout << std::to_string(encoder.Decode<double>(res)) << std::endl;

  HeKitFree(handle);
  HeKitFree(handle2);
}

TEST(HEU, API_OU_DestinationHeKit) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  std::cout << "pubkey: " << hekit->GetPublicKey()->ToString() << std::endl;
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  std::cout << "pubkey: " << dhekit->GetPublicKey()->ToString() << std::endl;
  assert(hekit->GetPublicKey()->ToString() ==
         dhekit->GetPublicKey()->ToString());

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_OU_Encrypt_Decrypt) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);

  auto len = 100000;
  auto data = new double[len];
  for (int i = 0; i < len; ++i) {
    data[i] = i + static_cast<double>(rand() % 10000) / 1000;
    /*if (i % 1000 == 0) {
      std::cout << "data[" << i << "]: " << std::to_string(data[i])
                << std::endl;
    }*/
  }
  auto res = new double[len];
  auto ciphers = new heu::lib::phe::Ciphertext[len];
  CheckCall(Encrypt(handle, data, len, ciphers), "Encrypt");
  CheckCall(Decrypt(handle, ciphers, len, res), "Decrypt");
  for (int i = 0; i < len; ++i) {
    if (data[i] != res[i]) {
      std::cout << "data[" << i << "]: " << std::to_string(data[i])
                << " ---- res[" << i << "]: " << std::to_string(res[i])
                << std::endl;
    }
  }
  double p = 2.687, q;
  heu::lib::phe::Ciphertext c;
  CheckCall(Encrypt(handle, p, &c), "Encrypt");
  CheckCall(Decrypt(handle, c, &q), "Decrypt");
  std::cout << "p: " << p << ", q: " << q << std::endl;

  HeKitFree(handle);
}

TEST(HEU, API_OU_Encrypt_Decrypt2) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);
  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);

  auto len = 100000;
  auto len2 = 10;
  auto data = new double[len];
  auto d1 = new double[len / 2];
  auto d2 = new double[len / 2];
  double **d3 = new double *[len / 2];
  double **d4 = new double *[len / 2];
  for (int i = 0; i < len; ++i) {
    data[i] = i + static_cast<double>(rand() % 10000) / 1000;
    if (i % 2 == 0) {
      d1[i / 2] = data[i];
      d3[i / 2] = new double[len2];
      for (int j = 0; j < len2; ++j) {
        d3[i / 2][j] = j + data[i];
      }
    } else {
      d2[i / 2] = data[i];
      d4[i / 2] = new double[len2];
      for (int j = 0; j < len2; ++j) {
        d4[i / 2][j] = 2 * j + data[i];
      }
    }
  }
  auto res = new double[len];
  auto ciphers = new heu::lib::phe::Ciphertext[len];
  CheckCall(Encrypt(handle, data, len, ciphers), "Encrypt");
  CheckCall(Decrypt(handle, ciphers, len, res), "Decrypt");

  auto encryptor = hekit->GetEncryptor();
  auto evaluator = hekit->GetEvaluator();
  auto decryptor = hekit->GetDecryptor();
  auto encoder = hekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto scale = 1e6;
  int128_t d10 = d1[0] * scale, d11 = d1[1] * scale, d20 = d2[0] * scale,
           d21 = d2[1] * scale;
  int128_t p1 = d10 << 64 | d11, p2 = d20 << 64 | d21;
  std::cout << "d1[0]: " << d1[0] << ", d1[1]: " << d1[1] << std::endl;
  std::cout << "d2[0]: " << d2[0] << ", d2[1]: " << d2[1] << std::endl;
  std::cout << "d10: " << d10 << ", d11: " << d11 << ", p1: " << p1
            << std::endl;
  std::cout << "d20: " << d20 << ", d21: " << d21 << ", p2: " << p2
            << std::endl;
  auto c1 = encryptor->Encrypt(encoder.Encode(p1));
  auto c2 = encryptor->Encrypt(encoder.Encode(p2));
  auto r1 = encoder.Decode<int128_t>(decryptor->Decrypt(c1));
  auto r2 = encoder.Decode<int128_t>(decryptor->Decrypt(c2));
  std::cout << "r10: " << (r1 >> 64) << ", r11: " << (int64_t)r1
            << ", r1: " << r1 << std::endl;
  std::cout << "r20: " << (r2 >> 64) << ", r21: " << (int64_t)r2
            << ", r2: " << r2 << std::endl;
  auto c = evaluator->Add(c1, c2);
  auto r = encoder.Decode<int128_t>(decryptor->Decrypt(c));
  std::cout << "d10 + d20: " << (d1[0] + d2[0])
            << ", d11 + d21: " << (d1[1] + d2[1]) << std::endl;
  std::cout << "r0: " << (r >> 64) << ", r1: " << (int64_t)r << ", r: " << r
            << std::endl;

  std::cout << "==============================================" << std::endl;
  heu::lib::phe::Ciphertext cipher;
  double a1, a2;
  CheckCall(Encrypt(handle, 1.2235, 2.4587, &cipher), "PairEncrypt");
  CheckCall(Decrypt(handle, cipher, &a1, &a2), "PairDecrypt");
  std::cout << "a1: " << a1 << ", a2: " << a2 << std::endl;

  std::cout << "==============================================" << std::endl;
  auto cs = new heu::lib::phe::Ciphertext[len / 2];
  auto res1 = new double[len / 2];
  auto res2 = new double[len / 2];
  CheckCall(Encrypt(handle, d1, d2, len / 2, cs), "PairEncrypt");
  CheckCall(Decrypt(handle, cs, len / 2, res1, res2), "PairDecrypt");
  std::cout << "res10: " << res1[0] << ", res11: " << res1[1] << std::endl;
  std::cout << "res20: " << res2[0] << ", res21: " << res2[1] << std::endl;

  std::cout << "==============================================" << std::endl;
  auto css = new heu::lib::phe::Ciphertext *[len / 2];
  auto rss1 = new double *[len / 2];
  auto rss2 = new double *[len / 2];
  for (int i = 0; i < len / 2; ++i) {
    css[i] = new heu::lib::phe::Ciphertext[len2];
    rss1[i] = new double[len2];
    rss2[i] = new double[len2];
  }
  CheckCall(Encrypt(handle, d3, d4, len / 2, len2, css), "PairEncrypt");
  CheckCall(Decrypt(handle, css, len / 2, len2, rss1, rss2), "PairDecrypt");
  std::cout << "d300: " << d3[0][0] << ", d301: " << d3[0][1] << std::endl;
  std::cout << "d400: " << d4[0][0] << ", d401: " << d4[0][1] << std::endl;
  std::cout << "r100: " << rss1[0][0] << ", r101: " << rss1[0][1] << std::endl;
  std::cout << "r200: " << rss2[0][0] << ", r201: " << rss2[0][1] << std::endl;

  HeKitFree(handle);
}

TEST(HEU, API_OU_Evaluate1) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  heu::lib::phe::Ciphertext c1, c2;
  CheckCall(DestinationHeKitEncrypt(dhandle, 12.3, &c1),
            "DestinationHeKitEncrypt");
  auto p = encoder.Encode(5);
  CheckCall(DestinationHeKitEncrypt(dhandle, 5, &c2),
            "DestinationHeKitEncrypt");

  auto c = AddCipher(dhandle, c1, c2);
  auto res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "AddCipher: " << res << std::endl;

  c = SubCipher(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "SubCipher: " << res << std::endl;

  c = AddPlain(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "AddPlain: " << res << std::endl;

  c = SubPlain(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "SubPlain: " << res << std::endl;

  c = MultiPlain(dhandle, c1, encoder2.Encode(5));
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "MultiPlain: " << res << std::endl;

  c = AddPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@AddPlain: " << res << std::endl;

  c = SubPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@SubPlain: " << res << std::endl;

  c = MultiPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@MultiPlain: " << res << std::endl;

  AddCipherInplace(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "AddCipherInplace: " << res << std::endl;

  SubCipherInplace(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "SubCipherInplace: " << res << std::endl;

  AddPlainInplace(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "AddPlainInplace: " << res << std::endl;

  SubPlainInplace(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "SubPlainInplace: " << res << std::endl;

  MultiPlainInplace(dhandle, c1, encoder2.Encode(5));
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "MultiPlainInplace: " << res << std::endl;
  CheckCall(DestinationHeKitEncrypt(dhandle, 12.3, &c1),
            "DestinationHeKitEncrypt");

  CheckCall(AddPlainInplace(dhandle, c1, 5), "AddPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@AddPlainInplace: " << res << std::endl;

  CheckCall(SubPlainInplace(dhandle, c1, 5), "SubPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@SubPlainInplace: " << res << std::endl;

  CheckCall(MultiPlainInplace(dhandle, c1, 5), "MultiPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@MultiPlainInplace: " << res << std::endl;

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_OU_Evaluate2) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto len = 10000, len2 = 10;
  double d1[len], d2[len], d3[len2];
  double *d4[len];
  heu::lib::phe::Ciphertext cs1[len], cs2[len];
  heu::lib::phe::Ciphertext *cs4[len];
  auto col_size = 50;
  auto cs5 = new heu::lib::phe::Ciphertext[len * col_size];
  for (int i = 0; i < len2; ++i) {
    d3[i] = i;
  }
  auto indexes = new long[len * col_size];
  for (int i = 0; i < len; ++i) {
    auto r = static_cast<double>(rand() % 10000) / 1000;
    d1[i] = i + r;
    d2[i] = 2 * i + r;
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
    for (int j = 0; j < col_size; ++j) {
      indexes[col_size * i + j] = i;
    }
    d4[i] = d3;
    cs4[i] = new heu::lib::phe::Ciphertext[len2];
  }

  CheckCall(Encrypt(handle, d1, len, cs1), "Encrypt1");
  CheckCall(Encrypt(handle, d2, len, cs2), "Encrypt2");
  for (int i = 0; i < len; ++i) {
    for (int j = 0; j < col_size; ++j) {
      cs5[col_size * i + j] = cs2[i];
    }
  }

  std::cout << "===================Inplace====================" << std::endl;
  CheckCall(AddCiphersInplace(dhandle, cs1, cs2, len), "AddCiphersInplace");
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  CheckCall(SubCiphersInplace(dhandle, cs1, cs2, len), "SubCiphersInplace");
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  std::cout << "===================Scatter====================" << std::endl;
  TIME_STAT(
      ScatterAddCiphersInplace(dhandle, cs1, cs5, indexes, len, len * col_size),
      ScatterAddCiphersInplace)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5005 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  TIME_STAT(
      ScatterSubCiphersInplace(dhandle, cs1, cs5, indexes, len, len * col_size),
      ScatterSubCiphersInplace)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5005 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  std::cout << "===================Axis0====================" << std::endl;
  CheckCall(Encrypt(handle, d4, len, len2, cs4), "Encrypts");
  TIME_STAT(AddCiphersInplaceAxis0(dhandle, cs1, cs4, len, len2),
            AddCiphersInplaceAxis0)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  TIME_STAT(SubCiphersInplaceAxis0(dhandle, cs1, cs4, len, len2),
            SubCiphersInplaceAxis0)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_OU_Evaluate3) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);
  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, hekit->GetPublicKey()->Serialize().size(),
                         &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);

  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto len = 10000;
  double d1[len], real[len], res[len];
  double *d2[len];
  int row_size[len];
  heu::lib::phe::Ciphertext cs1[len];
  heu::lib::phe::Ciphertext *cs2[len];
  for (int i = 0; i < len; ++i) {
    auto r = static_cast<double>(rand() % 10000) / 1000;
    d1[i] = i + r;
    real[i] = d1[i];
    row_size[i] = 100;
    d2[i] = new double[row_size[i]];
    for (int j = 0; j < row_size[i]; ++j) {
      r = static_cast<double>(rand() % 10000) / 1000;
      d2[i][j] = (j + 1) + r;
      real[i] += d2[i][j];
    }
    cs2[i] = new heu::lib::phe::Ciphertext[row_size[i]];
  }
  TIME_STAT(CheckCall(Encrypt(handle, d2, len, row_size, cs2, 100), "Encrypt1"),
            Encrypts);
  CheckCall(Encrypt(handle, d1, len, cs1), "Encrypt1");

  std::cout << "============AddCiphersInplaceAxis0_==============" << std::endl;
  TIME_STAT(AddCiphersInplaceAxis0_(dhandle, cs1, cs2, row_size, len, 100, 1);
            , AddCiphersInplaceAxis0_)
  CheckCall(Decrypt(handle, cs1, len, res), "Decrypt1");
  /*for (int i = 0; i < len; ++i) {
    if (i % 20 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- real["
                << i << "]: " << std::to_string(real[i]) << " ---- res[" << i
                << "]: " << std::to_string(res[i]) << std::endl;
    }
  }*/

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_OU_Evaluate4) {
  HeKitHandle handle;
  HeKitCreate(S_OU, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);

  auto len = 20000;
  int a[len], real_res = 0, eval_res = 0;
  heu::lib::phe::Ciphertext ciphers[len], out1, out2;
  for (int i = 0; i < len; ++i) {
    a[i] = i;
    real_res += i;
  }
  CheckCall(DestinationHeKitEncrypt(dhandle, a, len, ciphers, 1),
            "DHeKitEncrypt");
  TIME_STAT(SumCiphers(dhandle, ciphers, len, &out1, len + 1), SumCiphers1)
  CheckCall(Decrypt(handle, out1, &eval_res, 1), "Decrypt");
  std::cout << "real_res: " << real_res << ", eval_res: " << eval_res
            << std::endl;
  TIME_STAT(SumCiphers(dhandle, ciphers, len, &out2), SumCiphers2)
  CheckCall(Decrypt(handle, out2, &eval_res, 1), "Decrypt");
  std::cout << "real_res: " << real_res << ", eval_res: " << eval_res
            << std::endl;

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_OU_BatchEncoding) {
  auto scheme = heu::lib::phe::SchemaType::OU;
  auto he_kit_ = heu::lib::phe::HeKit(scheme, 2048);
  auto edr = he_kit_.GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto encryptor = he_kit_.GetEncryptor();
  auto evaluator = he_kit_.GetEvaluator();
  auto decryptor = he_kit_.GetDecryptor();
  heu::lib::phe::BatchEncoder batch_encoder(he_kit_.GetSchemaType());

  auto m0 = batch_encoder.Encode<int64_t>(-123, 123);
  auto ct0 = encryptor->Encrypt(m0);

  auto res = evaluator->Add(ct0, batch_encoder.Encode<int64_t>(23, 23));
  auto plain = decryptor->Decrypt(res);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 0>(plain)), -123 + 23);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 1>(plain)), 123 + 23);

  res = evaluator->Add(ct0, batch_encoder.Encode<int64_t>(-123, -456));
  decryptor->Decrypt(res, &plain);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 0>(plain)), -123 - 123);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 1>(plain)), 123 - 456);

  res = evaluator->Add(
      ct0, batch_encoder.Encode<int64_t>(std::numeric_limits<int64_t>::max(),
                                         std::numeric_limits<int64_t>::max()));
  decryptor->Decrypt(res, &plain);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 0>(plain)),
            -123LL + std::numeric_limits<int64_t>::max());
  EXPECT_EQ((batch_encoder.Decode<int64_t, 1>(plain)),
            std::numeric_limits<int64_t>::lowest() + 122);  // overflow

  // test big number
  ct0 = encryptor->Encrypt(
      batch_encoder.Encode<int64_t>(std::numeric_limits<int64_t>::lowest(),
                                    std::numeric_limits<int64_t>::max()));
  res = evaluator->Add(
      ct0, batch_encoder.Encode<int64_t>(std::numeric_limits<int64_t>::max(),
                                         std::numeric_limits<int64_t>::max()));
  decryptor->Decrypt(res, &plain);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 0>(plain)), -1);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 1>(plain)), -2);

  res = evaluator->Add(ct0, batch_encoder.Encode<int64_t>(-1, 1));
  decryptor->Decrypt(res, &plain);
  EXPECT_EQ((batch_encoder.Decode<int64_t, 0>(plain)),
            std::numeric_limits<int64_t>::max());
  EXPECT_EQ((batch_encoder.Decode<int64_t, 1>(plain)),
            std::numeric_limits<int64_t>::lowest());
}

TEST(HEU, API_ElGamal_HeKit) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);
  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);

  HeKitHandle handle2;
  std::size_t pub_key_size = hekit->GetPublicKey()->Serialize().size();
  std::size_t sec_key_size = hekit->GetSecretKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  SecKeyHandle skhandle;
  GetSecKey(hekit, &skhandle);
  HeKitCreateFromKeys(pkhandle, pub_key_size, skhandle, sec_key_size, &handle2);
  auto hekit2 = static_cast<heu::lib::phe::HeKit *>(handle2);

  auto encoder = hekit->GetEncoder<heu::lib::phe::PlainEncoder>(1e5);
  auto encryptor = hekit->GetEncryptor();
  auto evaluator = hekit2->GetEvaluator();
  auto decryptor = hekit->GetDecryptor();
  auto ct1 = encryptor->Encrypt(encoder.Encode(11122.22222));
  auto ct2 = encryptor->Encrypt(encoder.Encode(11133.33333));
  auto ct3 = evaluator->Add(ct1, ct2);
  auto res = decryptor->Decrypt(ct3);
  std::cout << std::to_string(encoder.Decode<double>(res)) << std::endl;

  HeKitFree(handle);
  HeKitFree(handle2);
}

TEST(HEU, API_ElGamal_DestinationHeKit) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  std::cout << "pubkey: " << hekit->GetPublicKey()->ToString() << std::endl;
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  std::cout << "pubkey: " << dhekit->GetPublicKey()->ToString() << std::endl;
  assert(hekit->GetPublicKey()->ToString() ==
         dhekit->GetPublicKey()->ToString());

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_ElGamal_Encrypt_Decrypt) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);

  auto len = 100000;
  auto data = new double[len];
  for (int i = 0; i < len; ++i) {
    data[i] = i + static_cast<double>(rand() % 10000) / 1000;
  }
  auto res = new double[len];
  auto ciphers = new heu::lib::phe::Ciphertext[len];
  CheckCall(Encrypt(handle, data, len, ciphers), "Encrypt");
  CheckCall(Decrypt(handle, ciphers, len, res, 1e6, 100), "Decrypt");
  for (int i = 0; i < len; ++i) {
    if (data[i] != res[i]) {
      std::cout << "data[" << i << "]: " << std::to_string(data[i])
                << " ---- res[" << i << "]: " << std::to_string(res[i])
                << std::endl;
    }
  }
  double p = 2.687, q;
  heu::lib::phe::Ciphertext c;
  CheckCall(Encrypt(handle, p, &c), "Encrypt");
  CheckCall(Decrypt(handle, c, &q), "Decrypt");
  std::cout << "p: " << p << ", q: " << q << std::endl;

  HeKitFree(handle);
}

TEST(HEU, API_ElGamal_Evaluate1) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  heu::lib::phe::Ciphertext c1, c2;
  CheckCall(DestinationHeKitEncrypt(dhandle, 12.3, &c1),
            "DestinationHeKitEncrypt");
  auto p = encoder.Encode(5);
  CheckCall(DestinationHeKitEncrypt(dhandle, 5, &c2),
            "DestinationHeKitEncrypt");

  auto c = AddCipher(dhandle, c1, c2);
  auto res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "AddCipher: " << res << std::endl;

  c = SubCipher(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "SubCipher: " << res << std::endl;

  c = AddPlain(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "AddPlain: " << res << std::endl;

  c = SubPlain(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "SubPlain: " << res << std::endl;

  c = MultiPlain(dhandle, c1, encoder2.Encode(5));
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "MultiPlain: " << res << std::endl;

  c = AddPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@AddPlain: " << res << std::endl;

  c = SubPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@SubPlain: " << res << std::endl;

  c = MultiPlain(dhandle, c1, 5);
  res = encoder.Decode<double>(decryptor->Decrypt(c));
  std::cout << "@MultiPlain: " << res << std::endl;

  AddCipherInplace(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "AddCipherInplace: " << res << std::endl;

  SubCipherInplace(dhandle, c1, c2);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "SubCipherInplace: " << res << std::endl;

  AddPlainInplace(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "AddPlainInplace: " << res << std::endl;

  SubPlainInplace(dhandle, c1, p);
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "SubPlainInplace: " << res << std::endl;

  MultiPlainInplace(dhandle, c1, encoder2.Encode(5));
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "MultiPlainInplace: " << res << std::endl;
  CheckCall(DestinationHeKitEncrypt(dhandle, 12.3, &c1),
            "DestinationHeKitEncrypt");

  CheckCall(AddPlainInplace(dhandle, c1, 5), "AddPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@AddPlainInplace: " << res << std::endl;

  CheckCall(SubPlainInplace(dhandle, c1, 5), "SubPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@SubPlainInplace: " << res << std::endl;

  CheckCall(MultiPlainInplace(dhandle, c1, 5), "MultiPlainInplace");
  res = encoder.Decode<double>(decryptor->Decrypt(c1));
  std::cout << "@MultiPlainInplace: " << res << std::endl;

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_ElGamal_Evaluate2) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);
  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto len = 100, len2 = 10;
  double d1[len], d2[len], d3[len2];
  double *d4[len];
  heu::lib::phe::Ciphertext cs1[len], cs2[len];
  heu::lib::phe::Ciphertext *cs4[len];
  auto col_size = 5;
  auto cs5 = new heu::lib::phe::Ciphertext[len * col_size];
  for (int i = 0; i < len2; ++i) {
    d3[i] = i;
  }
  auto indexes = new long[len * col_size];
  for (int i = 0; i < len; ++i) {
    auto r = static_cast<double>(rand() % 10000) / 1000;
    d1[i] = i + r;
    d2[i] = 2 * i + r;
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
    for (int j = 0; j < col_size; ++j) {
      indexes[col_size * i + j] = i;
    }
    d4[i] = d3;
    cs4[i] = new heu::lib::phe::Ciphertext[len2];
  }

  CheckCall(Encrypt(handle, d1, len, cs1), "Encrypt1");
  CheckCall(Encrypt(handle, d2, len, cs2), "Encrypt2");
  for (int i = 0; i < len; ++i) {
    for (int j = 0; j < col_size; ++j) {
      cs5[col_size * i + j] = cs2[i];
    }
  }

  std::cout << "===================Inplace====================" << std::endl;
  CheckCall(AddCiphersInplace(dhandle, cs1, cs2, len), "AddCiphersInplace");
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  CheckCall(SubCiphersInplace(dhandle, cs1, cs2, len), "SubCiphersInplace");
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  std::cout << "===================Scatter====================" << std::endl;
  TIME_STAT(
      ScatterAddCiphersInplace(dhandle, cs1, cs5, indexes, len, len * col_size),
      ScatterAddCiphersInplace)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5005 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  TIME_STAT(
      ScatterSubCiphersInplace(dhandle, cs1, cs5, indexes, len, len * col_size),
      ScatterSubCiphersInplace)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5005 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  std::cout << "===================Axis0====================" << std::endl;
  CheckCall(Encrypt(handle, d4, len, len2, cs4), "Encrypts");
  TIME_STAT(AddCiphersInplaceAxis0(dhandle, cs1, cs4, len, len2),
            AddCiphersInplaceAxis0)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }
  TIME_STAT(SubCiphersInplaceAxis0(dhandle, cs1, cs4, len, len2),
            SubCiphersInplaceAxis0)
  CheckCall(Decrypt(handle, cs1, len, d1), "Decrypt1");
  CheckCall(Decrypt(handle, cs2, len, d2), "Decrypt2");
  for (int i = 0; i < len; ++i) {
    if (i % 5000 == 0) {
      std::cout << "d1[" << i << "]: " << std::to_string(d1[i]) << " ---- d2["
                << i << "]: " << std::to_string(d2[i]) << std::endl;
    }
  }

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_ElGamal_Evaluate3) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);
  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, hekit->GetPublicKey()->Serialize().size(),
                         &dhandle);
  auto dhekit = static_cast<heu::lib::phe::DestinationHeKit *>(dhandle);

  auto encoder = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>();
  auto encoder2 = dhekit->GetEncoder<heu::lib::phe::PlainEncoder>(1);

  auto len = 1000;
  double d1[len], real[len], res[len];
  double *d2[len];
  int row_size[len];
  heu::lib::phe::Ciphertext cs1[len];
  heu::lib::phe::Ciphertext *cs2[len];
  for (int i = 0; i < len; ++i) {
    auto r = static_cast<double>(rand() % 10000) / 1000;
    d1[i] = i + r;
    real[i] = d1[i];
    row_size[i] = 100;
    d2[i] = new double[row_size[i]];
    for (int j = 0; j < row_size[i]; ++j) {
      d2[i][j] = (j + 1) + r;
      real[i] += d2[i][j];
    }
    cs2[i] = new heu::lib::phe::Ciphertext[row_size[i]];
  }
  TIME_STAT(CheckCall(Encrypt(handle, d2, len, row_size, cs2, 100), "Encrypt1"),
            Encrypts);
  CheckCall(Encrypt(handle, d1, len, cs1), "Encrypt1");

  std::cout << "============AddCiphersInplaceAxis0_==============" << std::endl;
  TIME_STAT(AddCiphersInplaceAxis0_(dhandle, cs1, cs2, row_size, len, 100, 1);
            , AddCiphersInplaceAxis0_)
  CheckCall(Decrypt(handle, cs1, len, res), "Decrypt1");

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}

TEST(HEU, API_ElGamal_Evaluate4) {
  HeKitHandle handle;
  HeKitCreate(S_ElGamal, 2048, &handle);

  auto hekit = static_cast<heu::lib::phe::HeKit *>(handle);
  auto decryptor = hekit->GetDecryptor();
  std::size_t size = hekit->GetPublicKey()->Serialize().size();
  PubKeyHandle pkhandle;
  GetPubKey(hekit, &pkhandle);
  DestinationHeKitHandle dhandle;
  DestinationHeKitCreate(pkhandle, size, &dhandle);

  auto len = 100;
  int a[len], real_res = 0, eval_res = 0;
  heu::lib::phe::Ciphertext ciphers[len], out1, out2;
  for (int i = 0; i < len; ++i) {
    a[i] = i;
    real_res += i;
  }
  CheckCall(DestinationHeKitEncrypt(dhandle, a, len, ciphers, 1),
            "DHeKitEncrypt");
  TIME_STAT(SumCiphers(dhandle, ciphers, len, &out1, len + 1), SumCiphers1)
  CheckCall(Decrypt(handle, out1, &eval_res, 1), "Decrypt");
  std::cout << "real_res: " << real_res << ", eval_res: " << eval_res
            << std::endl;
  TIME_STAT(SumCiphers(dhandle, ciphers, len, &out2), SumCiphers2)
  CheckCall(Decrypt(handle, out2, &eval_res, 1), "Decrypt");
  std::cout << "real_res: " << real_res << ", eval_res: " << eval_res
            << std::endl;

  free(pkhandle);
  DestinationHeKitFree(dhandle);
  HeKitFree(handle);
}
