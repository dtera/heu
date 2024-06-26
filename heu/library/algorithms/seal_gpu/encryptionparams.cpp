#include "encryptionparams.h"

using std::logic_error;

namespace seal_gpu {

const ParmsID parmsIDZero = util::HashFunction::hash_zero_block;

// void EncryptionParameters::save_members(ostream &stream) const
// {
//     // Throw exceptions on std::ios_base::badbit and std::ios_base::failbit
//     auto old_except_mask = stream.exceptions();
//     try
//     {
//         stream.exceptions(ios_base::badbit | ios_base::failbit);

//         uint64_t poly_modulus_degree64 =
//         static_cast<uint64_t>(poly_modulus_degree_); uint64_t
//         coeff_modulus_size64 = static_cast<uint64_t>(coeff_modulus_.size());
//         uint8_t scheme = static_cast<uint8_t>(scheme_);

//         stream.write(reinterpret_cast<const char *>(&scheme),
//         sizeof(uint8_t)); stream.write(reinterpret_cast<const char
//         *>(&poly_modulus_degree64), sizeof(uint64_t));
//         stream.write(reinterpret_cast<const char *>(&coeff_modulus_size64),
//         sizeof(uint64_t)); for (const auto &mod : coeff_modulus_)
//         {
//             mod.save(stream, compr_mode_type::none);
//         }

//         // Only BFV and BGV uses plain_modulus but save it in any case for
//         simplicity plain_modulus_.save(stream, compr_mode_type::none);
//     }
//     catch (const ios_base::failure &)
//     {
//         stream.exceptions(old_except_mask);
//         throw runtime_error("I/O error");
//     }
//     catch (...)
//     {
//         stream.exceptions(old_except_mask);
//         throw;
//     }
//     stream.exceptions(old_except_mask);
// }

// void EncryptionParameters::load_members(istream &stream, SEAL_MAYBE_UNUSED
// SEALVersion version)
// {
//     // Throw exceptions on std::ios_base::badbit and std::ios_base::failbit
//     auto old_except_mask = stream.exceptions();
//     try
//     {
//         stream.exceptions(ios_base::badbit | ios_base::failbit);

//         // Read the scheme identifier
//         uint8_t scheme;
//         stream.read(reinterpret_cast<char *>(&scheme), sizeof(uint8_t));

//         // This constructor will throw if scheme is invalid
//         EncryptionParameters parms(scheme);

//         // Read the poly_modulus_degree
//         uint64_t poly_modulus_degree64 = 0;
//         stream.read(reinterpret_cast<char *>(&poly_modulus_degree64),
//         sizeof(uint64_t));

//         // Only check for upper bound; lower bound is zero for
//         scheme_type::none if (poly_modulus_degree64 >
//         SEAL_POLY_MOD_DEGREE_MAX)
//         {
//             throw logic_error("poly_modulus_degree is invalid");
//         }

//         // Read the coeff_modulus size
//         uint64_t coeff_modulus_size64 = 0;
//         stream.read(reinterpret_cast<char *>(&coeff_modulus_size64),
//         sizeof(uint64_t));

//         // Only check for upper bound; lower bound is zero for
//         scheme_type::none if (coeff_modulus_size64 >
//         SEAL_COEFF_MOD_COUNT_MAX)
//         {
//             throw logic_error("coeff_modulus is invalid");
//         }

//         // Read the coeff_modulus
//         vector<Modulus> coeff_modulus;
//         for (uint64_t i = 0; i < coeff_modulus_size64; i++)
//         {
//             coeff_modulus.emplace_back();
//             coeff_modulus.back().load(stream);
//         }

//         // Read the plain_modulus
//         Modulus plain_modulus;
//         plain_modulus.load(stream);

//         // Supposedly everything worked so set the values of member variables
//         parms.set_poly_modulus_degree(safe_cast<size_t>(poly_modulus_degree64));
//         parms.set_coeff_modulus(coeff_modulus);

//         // Only BFV and BGV uses plain_modulus; set_plain_modulus checks that
//         for
//         // other schemes it is zero
//         parms.set_plain_modulus(plain_modulus);

//         // Set the loaded parameters
//         swap(*this, parms);

//         stream.exceptions(old_except_mask);
//     }
//     catch (const ios_base::failure &)
//     {
//         stream.exceptions(old_except_mask);
//         throw runtime_error("I/O error");
//     }
//     catch (...)
//     {
//         stream.exceptions(old_except_mask);
//         throw;
//     }
//     stream.exceptions(old_except_mask);
// }

void EncryptionParameters::computeParmsID() {
  size_t coeff_modulus_size = coeff_modulus_.size();

  size_t total_uint64_count = size_t(1) +  // scheme
                              size_t(1) +  // poly_modulus_degree
                              coeff_modulus_size + plain_modulus_.uint64Count();

  // FIXME: allocate related action
  uint64_t *param_data = new uint64_t[total_uint64_count];
  for (size_t i = 0; i < total_uint64_count; i++) param_data[i] = 0;
  uint64_t *param_data_ptr = param_data;

  // Write the scheme identifier
  *param_data_ptr++ = static_cast<uint64_t>(scheme_);

  // Write the poly_modulus_degree. Note that it will always be positive.
  *param_data_ptr++ = static_cast<uint64_t>(poly_modulus_degree_);

  for (const auto &mod : coeff_modulus_) {
    *param_data_ptr++ = mod.value();
  }

  util::setUint(plain_modulus_.data(), plain_modulus_.uint64Count(),
                param_data_ptr);
  param_data_ptr += plain_modulus_.uint64Count();

  util::HashFunction::hash(param_data, total_uint64_count, parms_id_);

  // Did we somehow manage to get a zero block as result? This is reserved for
  // plaintexts to indicate non-NTT-transformed form.
  if (parms_id_ == parmsIDZero) {
    throw logic_error("parms_id cannot be zero");
  }

  delete[] param_data;
}
}  // namespace seal_gpu
