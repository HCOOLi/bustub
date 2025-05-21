//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <chrono>
#include "common/exception.h"
#include "fmt/base.h"

namespace bustub {
auto LRUKNode::Evictable() const -> bool { return is_evictable_; }

auto LRUKNode::SetEvictable(bool evictable) { is_evictable_ = evictable; }

auto LRUKNode::AccessTimes() const -> size_t { return history_.size(); }
void LRUKNode::Access(size_t timestamp) { history_.push_back(timestamp); }

auto LRUKNode::KTimestamp() const -> size_t {
  auto iter = history_.rbegin();
  for (size_t i = 1; i < k_ && i < history_.size(); i++) {
    ++iter;
  }
  return *iter;
}

/**
 *
 * TODO(P1): Add implementation
 *
 * @brief a new LRUKReplacer.
 * @param num_frames the maximum number of frames the LRUReplacer will be required to store
 */
LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

/**
 * TODO(P1): Add implementation
 *
 * @brief Find the frame with largest backward k-distance and evict that frame. Only frames
 * that are marked as 'evictable' are candidates for eviction.
 *
 * A frame with less than k historical references is given +inf as its backward k-distance.
 * If multiple frames have inf backward k-distance, then evict frame whose oldest timestamp
 * is furthest in the past.
 *
 * Successful eviction of a frame should decrement the size of replacer and remove the frame's
 * access history.
 *
 * @return the frame ID if a frame is successfully evicted, or `std::nullopt` if no frames can be evicted.
 */
auto LRUKReplacer::Evict() -> std::optional<frame_id_t> {

  frame_id_t frame_id = 0;
  if (!history_.empty()) {
    for (auto iter = history_.begin(); iter != history_.end(); ++iter) {
      auto node = node_store_.at(*iter);  // Must in the set
      if (node->Evictable()) {
        frame_id = *iter;
        break;
      }
    }
  }
  fmt::println("{}",__LINE__);
  if (frame_id > 0) {
    fmt::println("{}",__LINE__);
    this->Remove(frame_id);
    return frame_id;
  }

  if (!k_history_.empty()) {
    fmt::println("{}",__LINE__);
    for (auto iter = k_history_.begin(); iter != k_history_.end(); ++iter) {
      auto node = node_store_.at(*iter);  // Must in the set
      if (node->Evictable()) {
        frame_id = *iter;
        break;
      }
    }
  }
  fmt::println("{}",__LINE__);
  if (frame_id > 0) {
    this->Remove(frame_id);
    return frame_id;
  }

  return std::nullopt;
}

/**
 * TODO(P1): Add implementation
 *
 * @brief Record the event that the given frame id is accessed at current timestamp.
 * Create a new entry for access history if frame id has not been seen before.
 *
 * If frame id is invalid (ie. larger than replacer_size_), throw an exception. You can
 * also use BUSTUB_ASSERT to abort the process if frame id is invalid.
 *
 * @param frame_id id of frame that received a new access.
 * @param access_type type of access that was received. This parameter is only needed for
 * leaderboard tests.
 */
void LRUKReplacer::RecordAccess(frame_id_t frame_id, AccessType access_type) {
   /// TODO latch
  auto now = std::chrono::system_clock::now().time_since_epoch();
  auto timestamp = static_cast<size_t>(now.count());
  fmt::println("access {},{}", frame_id, timestamp);
  auto node = node_store_.find(frame_id);
  // First
  if (node == node_store_.end()) {
    std::shared_ptr<LRUKNode> n = std::make_shared<LRUKNode>(k_, frame_id, timestamp);
    node_store_[frame_id] = n;
    history_.emplace_back(frame_id);
    return;
  }

  auto n = node_store_[frame_id];
  n->Access(timestamp);
  if (n->AccessTimes() < this->k_) {
    // Do nothing ?
  } else if (n->AccessTimes() == k_) {  // reach K times
    history_.remove(frame_id);          // O(N)
  } else {
    k_history_.remove(frame_id);  // O(N)
  }
  auto iter = k_history_.begin();
  for (; iter != k_history_.end(); ++iter) {
    if (node_store_[*iter]->KTimestamp() > n->KTimestamp()) {
      break;
    }
  }
  k_history_.insert(iter, frame_id);
}

/**
 *
 * @brief Toggle whether a frame is evictable or non-evictable. This function also
 * controls replacer's size. Note that size is equal to number of evictable entries.
 *
 * If a frame was previously evictable and is to be set to non-evictable, then size should
 * decrement. If a frame was previously non-evictable and is to be set to evictable,
 * then size should increment.
 *
 * If frame id is invalid, throw an exception or abort the process.
 *
 * For other scenarios, this function should terminate without modifying anything.
 *
 * @param frame_id id of frame whose 'evictable' status will be modified
 * @param set_evictable whether the given frame is evictable or not
 */
void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  auto n = node_store_.at(frame_id);

  if (!n->Evictable() && set_evictable) {
    curr_size_++;
  } else if (n->Evictable() && !set_evictable) {
    curr_size_--;
  }
  n->SetEvictable(set_evictable);
}

/**
 *
 * @brief Remove an evictable frame from replacer, along with its access history.
 * This function should also decrement replacer's size if removal is successful.
 *
 * Note that this is different from evicting a frame, which always remove the frame
 * with largest backward k-distance. This function removes specified frame id,
 * no matter what its backward k-distance is.
 *
 * If Remove is called on a non-evictable frame, throw an exception or abort the
 * process.
 *
 * If specified frame is not found, directly return from this function.
 *
 * @param frame_id id of frame to be removed
 */
void LRUKReplacer::Remove(frame_id_t frame_id) {
  if (node_store_.find(frame_id) != node_store_.end()) {
    node_store_.erase(frame_id);
    history_.remove(frame_id);
    k_history_.remove(frame_id);
    curr_size_--;
  }
}

/**
 *
 * @brief Return replacer's size, which tracks the number of evictable frames.
 *
 * @return size_t
 */
auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub
