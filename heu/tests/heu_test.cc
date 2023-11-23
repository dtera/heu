
#include <gtest/gtest.h>

#include "stopwatch.hpp"
#include "utils.h"

#include "heu/library/phe/encoding/encoding.h"
#include "heu/library/phe/phe.h"

void SerialDeserialize(const heu::lib::phe::SchemaType &schema) {
  heu::lib::phe::HeKit he_kit(schema);
  auto encryptor = he_kit.GetEncryptor();
  auto evaluator = he_kit.GetEvaluator();
  auto decryptor = he_kit.GetDecryptor();
  auto encoder = he_kit.GetEncoder<heu::lib::phe::PlainEncoder>(1e4);

  heu::lib::phe::Ciphertext cipher = encryptor->Encrypt(encoder.Encode(2.8));
  auto buf = cipher.Serialize();
  StopWatch sw;

  sw.Mark("Serialize");
  for (int i = 0; i < 1000000; ++i) {
    buf = cipher.Serialize();
  };
  sw.Print(&StopWatch::ShowTickMills, "Serialize");

  sw.Mark("Deserialize");
  for (int i = 0; i < 1000000; ++i) {
    cipher.Deserialize(buf);
  };
  sw.Print(&StopWatch::ShowTickMills, "Deserialize");

  std::cout << "cipher: " << cipher << std::endl;
  std::cout << "buf.size(): " << buf.size() << std::endl;
  auto p = decryptor->Decrypt(cipher);
  std::cout << "plaintext: " << encoder.Decode<double>(p) << std::endl;
}

TEST(HEU, OUSerialDeserialize) {
  SerialDeserialize(heu::lib::phe::SchemaType::OU);
}

TEST(HEU, ElGamalSerialDeserialize) {
  SerialDeserialize(heu::lib::phe::SchemaType::ElGamal);
}
