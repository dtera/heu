
#include <gtest/gtest.h>

#include "stopwatch.hpp"
#include "utils.h"

#include "heu/library/phe/encoding/encoding.h"
#include "heu/library/phe/phe.h"

#define HE_PREPARE(scheme)                \
  heu::lib::phe::HeKit he_kit(schema);    \
  auto encryptor = he_kit.GetEncryptor(); \
  auto evaluator = he_kit.GetEvaluator(); \
  auto decryptor = he_kit.GetDecryptor(); \
  auto encoder = he_kit.GetEncoder<heu::lib::phe::PlainEncoder>(1e4);

void SerialDeserialize(const heu::lib::phe::SchemaType &schema) {
  HE_PREPARE(schema);

  const heu::lib::phe::Plaintext pt = encoder.Encode(2.8);
  heu::lib::phe::Ciphertext cipher = encryptor->Encrypt(pt);
  auto buf = cipher.Serialize();
  StopWatch sw;

  sw.Mark("Serialize");
  for (int i = 0; i < 1000000; ++i) {
    buf = cipher.Serialize();
  }
  sw.Print(&StopWatch::ShowTickMills, "Serialize");

  sw.Mark("Deserialize");
  for (int i = 0; i < 1000000; ++i) {
    cipher.Deserialize(buf);
  }
  sw.PrintWithMills("Deserialize");

  std::cout << "cipher: " << cipher << std::endl;
  std::cout << "buf.size(): " << buf.size() << std::endl;
  auto p = decryptor->Decrypt(cipher);
  std::cout << "plaintext: " << encoder.Decode<double>(p) << std::endl;
}

void Add(const heu::lib::phe::SchemaType &schema) {
  HE_PREPARE(schema);

  heu::lib::phe::Ciphertext c1 = encryptor->Encrypt(encoder.Encode(2.8));
  heu::lib::phe::Ciphertext c2 = encryptor->Encrypt(encoder.Encode(0.012));
  heu::lib::phe::Ciphertext res;

  StopWatch sw;
  sw.Mark("Add");
  for (int i = 0; i < 1000000; ++i) {
    res = evaluator->Add(c1, c2);
  }
  sw.Print(&StopWatch::ShowTickMills, "Add");

  sw.Mark("AddInplace");
  for (int i = 0; i < 1000000; ++i) {
    evaluator->AddInplace(&c1, c2);
  }
  sw.PrintWithSeconds("AddInplace");

  auto p1 = decryptor->Decrypt(res);
  std::cout << "Add: " << encoder.Decode<double>(p1) << std::endl;
  auto p2 = decryptor->Decrypt(c1);
  std::cout << "AddInplace: " << encoder.Decode<double>(p2) << std::endl;
}

TEST(HEU, OUSerialDeserialize) {
  SerialDeserialize(heu::lib::phe::SchemaType::OU);
}

TEST(HEU, ElGamalSerialDeserialize) {
  SerialDeserialize(heu::lib::phe::SchemaType::ElGamal);
}

TEST(HEU, OUAdd) { Add(heu::lib::phe::SchemaType::OU); }

TEST(HEU, ElGamalAdd) { Add(heu::lib::phe::SchemaType::ElGamal); }
