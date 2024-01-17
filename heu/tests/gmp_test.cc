
#include <gmp.h>
#include <gtest/gtest.h>

#include <iostream>
#include <thread>

#include "stopwatch.hpp"

TEST(gmp, test1) {
  mpz_t t1, t2, t3, res;
  mpz_inits(t1, t2, t3, res, nullptr);
  mpz_set_si(t1, 2);
  mpz_set_si(t2, 101);
  mpz_set_si(t3, 7);
  mpz_add(res, t1, t2);
  std::cout << "t1 + t2 = " << mpz_get_si(res) << std::endl;
  mpz_sub(res, t1, t2);
  std::cout << "t1 - t2 = " << mpz_get_si(res) << std::endl;
  mpz_powm(res, t1, t2, t3);
  std::cout << "t1^t2 mod t3 = " << mpz_get_si(res) << std::endl;
  mpz_clears(t1, t2, t3, res, nullptr);
}

void gmp_mod_test(size_t key_size, size_t iter, StopWatch &sw) {
  auto mark = "[mod]" + std::to_string(key_size) + " bits";
  sw.Mark(mark);
  srand(arc4random());
  size_t base = 2, exp;
  mpz_t t1, t2, t3, p, q, n, r;
  mpz_inits(t1, t2, t3, p, q, n, r, nullptr);
  mpz_set_ui(t1, base);
  mpz_set_ui(t2, key_size - 1);
  mpz_pow_ui(p, t1, key_size - 1);
  mpz_pow_ui(q, t1, key_size - 1);
  for (int i = 0; i < iter; ++i) {
    exp = rand() * 100 + i;
    mpz_set_ui(t3, exp);
    mpz_add_ui(p, p, rand() * 10 + i);
    mpz_add_ui(q, q, rand() + i);
    mpz_mul(n, p, q);
    mpz_powm(r, t1, t3, n);
  }
  char modn[2048];
  std::cout << "n = " << mpz_get_str(modn, 10, n) << std::endl;
  std::cout << base << "^" << exp << " mod n = " << mpz_get_ui(r) << std::endl;
  mpz_clears(t1, t2, t3, p, q, n, r, nullptr);
  sw.PrintWithMills(mark);
}

TEST(gmp, test_mod) {
  size_t iter = 100000;
  StopWatch sw;
  gmp_mod_test(1024, iter, sw);
  gmp_mod_test(2048, iter, sw);
}
