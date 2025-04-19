//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// skiplist.cpp
//
// Identification: src/primer/skiplist.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/skiplist.h"
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "common/macros.h"
#include "fmt/core.h"

namespace bustub {

/** @brief Checks whether the container is empty. */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::Empty() -> bool { return size_ == 0; }

/** @brief Returns the number of elements in the skip list. */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::Size() -> size_t { return size_; }

/**
 * @brief Iteratively deallocate all the nodes.
 *
 * We do this to avoid stack overflow when the skip list is large.
 *
 * If we let the compiler handle the deallocation, it will recursively call the destructor of each node,
 * which could block up the stack.
 */
SKIPLIST_TEMPLATE_ARGUMENTS void SkipList<K, Compare, MaxHeight, Seed>::Drop() {
  for (size_t i = 0; i < MaxHeight; i++) {
    auto curr = std::move(header_->links_[i]);
    while (curr != nullptr) {
      // std::move sets `curr` to the old value of `curr->links_[i]`,
      // and then resets `curr->links_[i]` to `nullptr`.
      curr = std::move(curr->links_[i]);
    }
  }
}

/**
 * @brief Removes all elements from the skip list.
 *
 * Note: You might want to use the provided `Drop` helper function.
 */
SKIPLIST_TEMPLATE_ARGUMENTS void SkipList<K, Compare, MaxHeight, Seed>::Clear() {
  std::shared_lock lk(rwlock_);
  Drop();
  size_ = 0;
  height_=1;
}

/**
 * @brief Inserts a key into the skip list.
 *
 * Note: `Insert` will not insert the key if it already exists in the skip list.
 *
 * @param key key to insert.
 * @return true if the insertion is successful, false if the key already exists.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::Insert(const K &key) -> bool {
  std::unique_lock lk(rwlock_);
  auto current_node = header_;

  auto path = std::vector<std::shared_ptr<SkipNode>>(MaxHeight);
  for (int current_level = height_ - 1;current_level >= 0; current_level--) {
      auto next_node = current_node->Next(current_level);
      while (next_node != nullptr&&compare_(next_node->Key(), key)) {
        current_node = next_node;
        next_node = current_node->Next(current_level);
      }
      if (next_node != nullptr&&!compare_(next_node->Key(),key)&&!compare_(key,next_node->Key())) {
        // already exists.
        return false;
      }
      path[current_level]=current_node;
    }

  auto new_height = RandomHeight();
  if (new_height > height_) {
    for (int current_level = height_ ; current_level < static_cast<int>(new_height); current_level++) {
       path[current_level]=header_;
    }
    height_=new_height;
  }


  auto new_node = std::make_shared<SkipNode>( new_height,key);

  for (auto h = 0; h < static_cast<int>(new_height); h++) {
      auto last_node = path[h];
      new_node->links_[h] = last_node->links_[h];
      last_node->links_[h] = new_node;
    }

  size_++;
  return true;
}

/**
 * @brief Erases the key from the skip list.
 *
 * @param key key to erase.
 * @return bool true if the element got erased, false otherwise.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::Erase(const K &key) -> bool {
  std::unique_lock lk(rwlock_);
  auto current_node = header_;

  auto path = std::vector<std::shared_ptr<SkipNode>>(MaxHeight);
  for (int current_level = height_ - 1;current_level >= 0; current_level--) {
    auto next_node = current_node->Next(current_level);
    while (next_node != nullptr&&compare_(next_node->Key(), key)) {
      current_node = next_node;
      next_node = current_node->Next(current_level);
    }
    path[current_level]=current_node;
  }
  current_node = current_node->Next(0);
  if (current_node != nullptr&&!compare_(current_node->Key(),key)&&!compare_(key,current_node->Key())) {
    for (auto h = 0; h < static_cast<int>(height_); h++) {
      if (path[h]!=current_node) {
        break;
      }
      path[h]->SetNext(h,current_node->Next(h));
    }
    //
    size_--;
    if (height_ > 1) {
      for (int current_level = height_ - 1; current_level >= 0; current_level--) {
        if (header_->links_[current_level] != nullptr) {
          break;
        }
        height_--;
      }
    }
    return true;
  }
  return false;

}

/**
 * @brief Checks whether a key exists in the skip list.
 *
 * @param key key to look up.
 * @return bool true if the element exists, false otherwise.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::Contains(const K &key) -> bool {
  // Following the standard library: Key `a` and `b` are considered equivalent if neither compares less
  // than the other: `!compare_(a, b) && !compare_(b, a)`.
  std::shared_lock lk(rwlock_);
  auto current_node = header_;
  for (int current_level = height_ - 1;current_level >= 0; current_level--) {
    auto next_node = current_node->Next(current_level);
    while (next_node != nullptr&&compare_(next_node->Key(), key)) {
      current_node = next_node;
      next_node = current_node->Next(current_level);
    }
    if (next_node != nullptr&&!compare_(next_node->Key(),key)&&!compare_(key,next_node->Key())) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Prints the skip list for debugging purposes.
 *
 * Note: You may modify the functions in any way and the output is not tested.
 */
SKIPLIST_TEMPLATE_ARGUMENTS void SkipList<K, Compare, MaxHeight, Seed>::Print() {
  std::shared_lock lk(rwlock_);
  auto node = header_->Next(LOWEST_LEVEL);
  while (node != nullptr) {
    fmt::println("Node {{ key: {}, height: {} }}", node->Key(), node->Height());
    node = node->Next(LOWEST_LEVEL);
  }
}

/**
 * @brief Generate a random height. The height should be capped at `MaxHeight`.
 * Note: we implement/simulate the geometric process to ensure platform independence.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::RandomHeight() -> size_t {
  // Branching factor (1 in 4 chance), see Pugh's paper.
  static constexpr unsigned int branching_factor = 4;
  // Start with the minimum height
  size_t height = 1;
  while (height < MaxHeight && (rng_() % branching_factor == 0)) {
    height++;
  }
  return height;
}

/**
 * @brief Gets the current node height.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::SkipNode::Height() const -> size_t {
  return links_.size();
}

/**
 * @brief Gets the next node by following the link at `level`.
 *
 * @param level index to the link.
 * @return std::shared_ptr<SkipNode> the next node, or `nullptr` if such node does not exist.
 */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::SkipNode::Next(size_t level) const
    -> std::shared_ptr<SkipNode> {
  return links_[level];
}

/**
 * @brief Set the `node` to be linked at `level`.
 *
 * @param level index to the link.
 */
SKIPLIST_TEMPLATE_ARGUMENTS void SkipList<K, Compare, MaxHeight, Seed>::SkipNode::SetNext(
    size_t level, const std::shared_ptr<SkipNode> &node) {
  links_[level] = node;
}

/** @brief Returns a reference to the key stored in the node. */
SKIPLIST_TEMPLATE_ARGUMENTS auto SkipList<K, Compare, MaxHeight, Seed>::SkipNode::Key() const -> const K & {
  return key_;
}

// Below are explicit instantiation of template classes.
template class SkipList<int>;
template class SkipList<std::string>;
template class SkipList<int, std::greater<>>;
template class SkipList<int, std::less<>, 8>;

}  // namespace bustub
