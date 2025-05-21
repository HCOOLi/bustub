//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hyperloglog.cpp
//
// Identification: src/primer/hyperloglog.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/hyperloglog.h"

#include <utility>

namespace bustub {

/** @brief Parameterized constructor. */
template <typename KeyType>
HyperLogLog<KeyType>::HyperLogLog(int16_t n_bits) : cardinality_(0), n_bits_(n_bits) {
  if ( n_bits<=0) {
    n_bits=-n_bits;
  }
  register_=std::vector<int>(1<<n_bits,0);
}

/**
 * @brief Function that computes binary.
 *
 * @param[in] hash
 * @returns binary of a given hash
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeBinary(const hash_t &hash) const -> std::bitset<BITSET_CAPACITY> {
    std::bitset<BITSET_CAPACITY> set;
    hash_t value= hash;
    for (int i = 0; i < BITSET_CAPACITY; i++) {
      if ( value >> i &1){
        set.set(i);
      }
    }
  return set;
}

/**
 * @brief Function that computes leading zeros.
 *
 * @param[in] bset - binary values of a given bitset
 * @returns leading zeros of given binary set
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::PositionOfLeftmostOne(const std::bitset<BITSET_CAPACITY> &bset) const -> uint64_t {
  for (int i = 0; i < BITSET_CAPACITY - n_bits_; i++) {
    if (bset[BITSET_CAPACITY - n_bits_ - 1 - i]) {
      return i;
    }
  }
  return BITSET_CAPACITY;
}

/**
 * @brief Adds a value into the HyperLogLog.
 *
 * @param[in] val - value that's added into hyperloglog
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::AddElem(KeyType val) -> void {
    hash_t h= CalculateHash(std::move(val));
    auto x = PositionOfLeftmostOne(ComputeBinary(h));
  auto p = h >> (BITSET_CAPACITY-n_bits_);
  fmt::println("h={},x={},p={}",h,x,p);
  this->register_[p]= std::max(this->register_[p], int(x+1));
}

/**
 * @brief Function that computes cardinality.
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeCardinality() -> void {
  cardinality_ = 0;
  double c = 0;
  for (auto x:this->register_) {
    if (x !=0) {
      c += 1.0/pow(2,x);
    }

  }
  auto n =register_.size();
  c= 0.79402*n*n/c;
  cardinality_ = std::floor(c);
}

template class HyperLogLog<int64_t>;
template class HyperLogLog<std::string>;

}  // namespace bustub
