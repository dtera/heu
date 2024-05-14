#pragma once

#include "batchencoder_cuda.cuh"
#include "ciphertext_cuda.cuh"
#include "ckks_cuda.cuh"
#include "context_cuda.cuh"
#include "decryptor_cuda.cuh"
#include "encryptionparams_cuda.cuh"
#include "encryptor_cuda.cuh"
#include "evaluator_cuda.cuh"
#include "galoiskeys_cuda.cuh"
#include "keygenerator_cuda.cuh"
#include "kswitchkeys_cuda.cuh"
#include "plaintext_cuda.cuh"
#include "publickey_cuda.cuh"
#include "relinkeys_cuda.cuh"
#include "secretkey_cuda.cuh"
#include "utils/rns_cuda.cuh"

namespace seal_gpun {
using seal_gpu::CoeffModulus;
using seal_gpu::Modulus;
using seal_gpu::ParmsID;
using seal_gpu::PlainModulus;
using seal_gpu::SchemeType;
using seal_gpu::SecurityLevel;
using EncryptionParameters = seal_gpu::EncryptionParametersCuda;
using SEALContext = seal_gpu::SEALContextCuda;
using Plaintext = seal_gpu::PlaintextCuda;
using Ciphertext = seal_gpu::CiphertextCuda;
using Encryptor = seal_gpu::EncryptorCuda;
using Decryptor = seal_gpu::DecryptorCuda;
using Evaluator = seal_gpu::EvaluatorCuda;
using KeyGenerator = seal_gpu::KeyGeneratorCuda;
using PublicKey = seal_gpu::PublicKeyCuda;
using SecretKey = seal_gpu::SecretKeyCuda;
using KSwitchKeys = seal_gpu::KSwitchKeysCuda;
using RelinKeys = seal_gpu::RelinKeysCuda;
using GaloisKeys = seal_gpu::GaloisKeysCuda;
using CKKSEncoder = seal_gpu::CKKSEncoderCuda;
using BatchEncoder = seal_gpu::BatchEncoderCuda;
using KernelProvider = seal_gpu::KernelProvider;
using LWECiphertext = seal_gpu::LWECiphertextCuda;
}  // namespace seal_gpun
