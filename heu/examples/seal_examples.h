// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "seal/seal.h"
#include "utils.h"

/*
Helper function: Prints the parameters in a SEALContext.
*/
inline void print_parameters(const seal::SEALContext &context) {
  auto &context_data = *context.key_context_data();

  /*
  Which scheme are we using?
  */
  std::string scheme_name;
  switch (context_data.parms().scheme()) {
    case seal::scheme_type::bfv:
      scheme_name = "BFV";
      break;
    case seal::scheme_type::ckks:
      scheme_name = "CKKS";
      break;
    case seal::scheme_type::bgv:
      scheme_name = "BGV";
      break;
    default:
      throw std::invalid_argument("unsupported scheme");
  }
  std::cout << "/" << std::endl;
  std::cout << "| Encryption parameters :" << std::endl;
  std::cout << "|   scheme: " << scheme_name << std::endl;
  std::cout << "|   poly_modulus_degree: "
            << context_data.parms().poly_modulus_degree() << std::endl;

  /*
  Print the size of the true (product) coefficient modulus.
  */
  std::cout << "|   coeff_modulus size: ";
  std::cout << context_data.total_coeff_modulus_bit_count() << " (";
  auto coeff_modulus = context_data.parms().coeff_modulus();
  std::size_t coeff_modulus_size = coeff_modulus.size();
  for (std::size_t i = 0; i < coeff_modulus_size - 1; i++) {
    std::cout << coeff_modulus[i].bit_count() << " + ";
  }
  std::cout << coeff_modulus.back().bit_count();
  std::cout << ") bits" << std::endl;

  /*
  For the BFV scheme print the plain_modulus parameter.
  */
  if (context_data.parms().scheme() == seal::scheme_type::bfv) {
    std::cout << "|   plain_modulus: "
              << context_data.parms().plain_modulus().value() << std::endl;
  }

  std::cout << "\\" << std::endl;
}

/*
Helper function: Prints the `parms_id' to std::ostream.
*/
inline std::ostream &operator<<(std::ostream &stream,
                                seal::parms_id_type parms_id) {
  /*
  Save the formatting information for std::cout.
  */
  std::ios old_fmt(nullptr);
  old_fmt.copyfmt(std::cout);

  stream << std::hex << std::setfill('0') << std::setw(16) << parms_id[0] << " "
         << std::setw(16) << parms_id[1] << " " << std::setw(16) << parms_id[2]
         << " " << std::setw(16) << parms_id[3] << " ";

  /*
  Restore the old std::cout formatting.
  */
  std::cout.copyfmt(old_fmt);

  return stream;
}

/*
Helper function: Convert a value into a hexadecimal string, e.g., uint64_t(17)
--> "11".
*/
inline std::string uint64_to_hex_string(std::uint64_t value) {
  return seal::util::uint_to_hex_string(&value, std::size_t(1));
}
