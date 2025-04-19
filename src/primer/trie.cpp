//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie.cpp
//
// Identification: src/primer/trie.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/trie.h"

#include <type/value.h>

#include <string_view>
#include "common/exception.h"

namespace bustub {

/**
 * @brief Get the value associated with the given key.
 * 1. If the key is not in the trie, return nullptr.
 * 2. If the key is in the trie but the type is mismatched, return nullptr.
 * 3. Otherwise, return the value.
 */
template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  if (root_==nullptr) return nullptr;
  if (key.empty()) {
    if (root_->is_value_node_) {
      std::shared_ptr< const TrieNodeWithValue<T>> value_node_ptr = std::dynamic_pointer_cast<const TrieNodeWithValue<T>>(root_);
      return value_node_ptr->value_.get();
    }
    return nullptr;
  }
  std::shared_ptr<const TrieNode> cur_ = root_;
  char last_char = 0;
  for (char c : key) {
    if (cur_->children_.find(c) == cur_->children_.end()) {
      return nullptr;
    }
    last_char = c;
    cur_ = cur_->children_.at(c);
  }
  if (last_char == key.back()) {
    if (cur_->is_value_node_) {
      std::shared_ptr< const TrieNodeWithValue<T>> value_node_ptr = std::dynamic_pointer_cast<const TrieNodeWithValue<T>>(cur_);
      return value_node_ptr->value_.get();
    }
  }
  return nullptr;

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.
}

/**
 * @brief Put a new key-value pair into the trie. If the key already exists, overwrite the value.
 * @return the new trie.
 */
template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.

  std::shared_ptr<const TrieNode> cur_ = root_;
  std::stack<std::pair<char,std::shared_ptr<const TrieNode>>> stack;
  for (char c : key) {
    if (cur_==nullptr || (cur_->children_.find(c) == cur_->children_.end())) {
      stack.emplace(c,nullptr);
    }else {
      stack.emplace(c,cur_);
      cur_ = cur_->children_.at(c);
    }
  }

  std::shared_ptr<const TrieNode> last_node_ptr =  std::make_shared<TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));;
  while (!stack.empty()) {
    auto [c,cur_] = stack.top();
    stack.pop();
    TrieNode last_node ;
    if (cur_ ==nullptr){
      last_node = TrieNode();
    }else {
      last_node = TrieNode(*cur_);
    }
    last_node.children_[c] = last_node_ptr;
    last_node_ptr = std::make_shared<const TrieNode>(last_node);
  }

  return Trie(last_node_ptr);


  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.
}

/**
 * @brief Remove the key from the trie.
 * @return If the key does not exist, return the original trie. Otherwise, returns the new trie.
 */
auto Trie::Remove(std::string_view key) const -> Trie {
  throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value anymore,
  // you should convert it to `TrieNode`. If a node doesn't have children anymore, you should remove it.
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
