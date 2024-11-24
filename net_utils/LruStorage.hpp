#pragma once

#include <cassert>
#include <list>
#include <unordered_map>
#include <utility>


namespace nut {


template<typename Key_, typename Value_, typename KeyProj_ = Key_>
class LruStorage final {
public:
  using key_type      = Key_;
  using value_type    = Value_;
  using key_proj_type = KeyProj_;

public:
  LruStorage(std::size_t capacity, bool reserve = true)
      : capacity_{capacity} {
    assert(capacity_ > 0);
    reset(capacity_, reserve);
  }


  [[nodiscard]] std::size_t size() const noexcept {
    assert(map_.size() <= capacity_);
    assert(map_.size() == list_.size());
    return map_.size();
  }


  [[nodiscard]] bool empty() const noexcept {
    return map_.empty();
  }


  template<typename Key>
  [[nodiscard]] value_type const* get(Key&& key) noexcept {
    auto const it = map_.find(key);// c++20 heterogeneous comparator
    if (it == map_.cend()) {
      return nullptr;
    }
    move_to_front(it);
    return &it->second->value;
  }


  template<typename Key, typename... VArgs>
  value_type& put(Key&& key, VArgs&&... v_args) {
    if (auto const it = map_.find(key); it != map_.cend()) {
      assert(it->second != list_.cend());
      it->second->value = value_type{std::forward<VArgs>(v_args)...};
      move_to_front(it);
      return it->second->value;
    }

    if (size() == capacity_) {
      map_.erase(list_.back().key);
      list_.pop_back();
    }
    list_.emplace_front(Node{std::forward<Key>(key), {std::forward<VArgs>(v_args)...}});
    auto const [it, f] = map_.emplace(list_.front().key, list_.begin());
    assert(f);
    return it->second->value;
  }


  void clear() noexcept {
    reset(capacity_);
  }


  void reset(std::size_t capacity, bool reserve = true) noexcept {
    list_.clear();
    map_.clear();
    capacity_ = capacity;
    if (reserve) {
      map_.reserve(capacity);
    }
  }

private:
  struct Node final {
    key_type   key;
    value_type value;
  };

  using List    = std::list<Node>;
  using HashMap = std::unordered_map<key_proj_type, typename List::iterator>;// unordered_set c++20 heterogeneous comparator

private:
  void move_to_front(typename HashMap::const_iterator it) noexcept {
    list_.splice(list_.cbegin(), list_, it->second);
  }

private:
  List        list_;
  HashMap     map_;
  std::size_t capacity_;
}

;
}// namespace nut