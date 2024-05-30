// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "seal_examples_gpu.h"

using namespace std;
using namespace seal_gpun;

int main() {
  /* 计算 （1.23 x 4.56）^ 2 +（7.89 x 10.11）^ 2 +（12.13 x 14.15）^ 2
   * 需要打包旋转 */
  /* 首先设置加密方案的参数 */
  KernelProvider::initialize();
  EncryptionParameters parms(SchemeType::ckks);
  size_t poly_modulus_degree = 8192;
  parms.setPolyModulusDegree(poly_modulus_degree);
  parms.setCoeffModulus(
      CoeffModulus::Create(poly_modulus_degree, {40, 40, 40, 40, 40}));
  SEALContext context(parms);

  /* 生成密钥与加密、解密 */
  KeyGenerator keygen(context);
  SecretKey secret_key = keygen.secretKey();
  PublicKey public_key;
  keygen.createPublicKey(public_key);
  RelinKeys relin_keys;
  keygen.createRelinKeys(relin_keys);
  GaloisKeys gal_keys;
  keygen.createGaloisKeys(gal_keys);
  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  /* 使用 CKKSEncoder 批处理并编码 */
  CKKSEncoder encoder(context);
  auto slots = encoder.slotCount();
  /* 设置 scale 缩放的值 */
  double scale = pow(2.0, 40);

  /* 初始化向量 */
  vector<double> vx1(slots, 0.0);
  vector<double> vx2(slots, 0.0);
  /* 将 1.23，7.89，12.13 添加到向量 */
  vx1[0] = 1.23;
  vx1[1] = 7.89;
  vx1[2] = 12.13;
  /* 将 4.56，10.11，14.15 添加到向量 */
  vx2[0] = 4.56;
  vx2[1] = 10.11;
  vx2[2] = 14.15;
  double real = 0;
  for (int i = 0; i < 3; i++) {
    double t = vx1[i] * vx2[i];
    real += t * t;
  }

  cout << "待编码的向量vx1:";
  print_vector(vx1);
  cout << "待编码的向量vx2:";
  print_vector(vx2);

  /* 编码并加密 */
  Plaintext plain_vx1, plain_vx2;
  Ciphertext encrypted_vx1, encrypted_vx2;
  encoder.encodePolynomial(vx1, scale, plain_vx1);
  encoder.encodePolynomial(vx2, scale, plain_vx2);
  encryptor.encrypt(plain_vx1, encrypted_vx1);
  encryptor.encrypt(plain_vx2, encrypted_vx2);

  /* 计算 （1.23 x 4.56）^ 2 +（7.89 x 10.11）^ 2 +（12.13 x 14.15）^ 2 */
  // 5.6088 ** 2 + 79.7679 ** 2 + 171.6395 ** 2
  Ciphertext mul;
  evaluator.multiply(encrypted_vx1, encrypted_vx2, mul);
  evaluator.relinearizeInplace(mul, relin_keys);
  evaluator.rescaleToNextInplace(mul);
  evaluator.squareInplace(mul);
  evaluator.relinearizeInplace(mul, relin_keys);
  evaluator.rescaleToNextInplace(mul);

  /* 旋转并相加，计算结果 */
  Ciphertext total = mul;
  for (long i = 1; i < 3; ++i) {
    Ciphertext tmp;
    evaluator.rotateVector(mul, i, gal_keys, tmp);
    evaluator.addInplace(total, tmp);
  }

  /* 解密 */
  /* 将密文解密解码至 vector */
  Plaintext total_plain;
  vector<double> result;
  decryptor.decrypt(total, total_plain);
  encoder.decodePolynomial(total_plain, result);

  /* 打印结果 */
  cout << endl << "解密数组: ";
  print_vector(result);  // 直接输出解密向量
  cout << "解密结果: " << result[0] << endl;
  cout << "实际结果: " << real << endl;

  return 0;
}
