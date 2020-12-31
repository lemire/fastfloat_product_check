
#include "fast_float/fast_float.h"
#include <iostream>



template <typename binary>
bool is_indeterminate(int64_t q, uint64_t w)  noexcept  {
  if ((w == 0) || (q < binary::smallest_power_of_ten())) { return false; }
  if (q > binary::largest_power_of_ten()) { return false; }
  int lz = fast_float::leading_zeroes(w);
  w <<= lz;
  fast_float::value128 product = fast_float::compute_product_approximation<binary::mantissa_explicit_bits() + 3>(q, w);
  if(product.low == 0xFFFFFFFFFFFFFFFF) { //  could guard it further
    // In some very rare cases, this could happen, in which case we might need a more accurate
    // computation that what we can provide cheaply. This is very, very unlikely.
    //
    const bool inside_safe_exponent = (q >= -27) && (q <= 55); // always good because 5**q <2**128 when q>=0, 
    // and otherwise, for q<0, we have 5**-q<2**64 and the 128-bit reciprocal allows for exact computation.
    if(!inside_safe_exponent) {
        return true;
    }
  }
  return false;
}

// return 0 on success, otherwise returns the decimal signficand
uint64_t check_power(int64_t q, uint64_t maxw) {
    std::cout << " checking q = "<< q << std::endl;
    // 9**10 == 3486784401
    for(uint64_t w = 1; w <= maxw; w++) {
        if(is_indeterminate<fast_float::binary_format<double>>(q,w)) {
            return w;
        }
    }
    return 0;
}

std::pair<int64_t, uint64_t> check_all_powers(uint64_t maxw) {
  for(int64_t q = fast_float::binary_format<double>::smallest_power_of_ten(); q < -27; q++) {
      uint64_t w = check_power(q, maxw);
      if(w != 0) { return std::make_pair(q,w); }
  }
  for(int64_t q = 56; q <= fast_float::binary_format<double>::largest_power_of_ten(); q++) {
      uint64_t w = check_power(q, maxw);
      if(w != 0) { return std::make_pair(q,w); }
  }
  return std::make_pair(0,0);
}

int main() {
    uint64_t number_of_digits = 11;
    std::cout << "number_of_digits = " << number_of_digits << std::endl;
    uint64_t maxw = 9;
    {
        uint64_t digits = 1;
        while (digits < number_of_digits) {
            maxw *= 9;
            digits++;
        }
    }
    std::cout << "maximal decimal significand " << maxw << std::endl;
    auto [q,w] = check_all_powers(maxw);
    if(w != 0) {
        std::cout << " w = " << w << " q = " << q << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}