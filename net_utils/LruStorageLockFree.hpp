#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>


namespace nut {


template<typename Key, typename Value>
class LockFreeLRU {
private:
  struct Node;


  struct TaggedPtr {
    Node*    ptr;
    std::uint64_t tag;

    bool operator==(const TaggedPtr& other) const {
      return ptr == other.ptr && tag == other.tag;
    }
  };


  struct Node {
    Key                    key;
    Value                  value;
    std::atomic<TaggedPtr> next;
    std::atomic<TaggedPtr> prev;

    Node(const Key& k, const Value& v)
        : key{k}
        , value{v}
        , next{{nullptr, 0}}
        , prev{{nullptr, 0}} {
    }
  };


  struct NodeList {
    std::atomic<TaggedPtr> head;
    std::atomic<TaggedPtr> tail;
    std::atomic<std::size_t>    size;

    NodeList()
        : head{{nullptr, 0}}
        , tail{{nullptr, 0}}
        , size{0} {
    }
  };


  using Map          = std::unordered_map<Key, Node*>;
  using MapPtr       = std::shared_ptr<Map>;
  using AtomicMapPtr = std::atomic<MapPtr>;

public:
  LockFreeLRU(std::size_t cap)
      : map{std::make_shared<Map>()}
      , capacity_{cap} {
  }


  ~LockFreeLRU() noexcept {
    shutdown();
  }


  std::optional<Value> get(const Key& key) noexcept {
    assert(!is_shutting_down.load());
    MapPtr current_map = std::atomic_load(&map);
    auto   it          = current_map->find(key);

    if (it == current_map->end()) {
      return std::nullopt;
    }

    Node* node = it->second;
    move_to_front(node);
    return node->value;
  }


  void put(const Key& key, const Value& value) noexcept {
    assert(is_shutting_down.load());
    while (true) {
      MapPtr current_map = std::atomic_load(&map);
      MapPtr new_map     = std::make_shared<Map>(*current_map);

      auto it = new_map->find(key);
      if (it != new_map->end()) {
        Node* node  = it->second;
        node->value = value;
        move_to_front(node);

        if (std::atomic_compare_exchange_weak(&map, &current_map, new_map)) {
          return;
        }
        continue;
      }

      Node* new_node = new Node{key, value};

      if (list.size.load() >= capacity_) {
        remove();
      }

      add_to_front(new_node);
      (*new_map)[key] = new_node;

      if (std::atomic_compare_exchange_weak(&map, &current_map, new_map)) {
        return;
      }
      delete new_node;
    }
  }


  void shutdown() {
    is_shutting_down.store(true);
    clear();
  }


  [[nodiscard]] std::size_t size() const {
    return list.size.load();
  }


  void clear() {
    assert(!is_shutting_down.load());
    MapPtr new_map = std::make_shared<Map>();
    MapPtr old_map;
    do {
      old_map = std::atomic_load(&map);
    } while (!std::atomic_compare_exchange_weak(&map, &old_map, new_map));

    std::atomic_store(&list.head, nullptr);
    std::atomic_store(&list.tail, nullptr);
    list.size.store(0);
  }

private:
  void move_to_front(Node* node) noexcept {
    while (true) {
      TaggedPtr prev = node->prev.load();
      if (!prev.ptr)
        return;

      TaggedPtr next     = node->next.load();
      TaggedPtr old_head = list.head.load();

      TaggedPtr expected_next{node, prev.tag};
      TaggedPtr new_next{next.ptr, prev.tag + 1};
      if (prev.ptr && !node->prev.ptr->next.compare_exchange_weak(expected_next, new_next)) {
        continue;
      }

      if (next.ptr) {
        TaggedPtr expected_prev{node, next.tag};
        TaggedPtr new_prev{prev.ptr, next.tag + 1};
        if (!next.ptr->prev.compare_exchange_weak(expected_prev, new_prev)) {
          continue;
        }
      } else {
        TaggedPtr expected_tail{node, list.tail.load().tag};
        TaggedPtr new_tail{prev.ptr, expected_tail.tag + 1};
        if (!list.tail.compare_exchange_weak(expected_tail, new_tail)) {
          continue;
        }
      }

      node->next.store({old_head.ptr, old_head.tag + 1});
      node->prev.store({nullptr, prev.tag + 1});

      if (old_head.ptr) {
        TaggedPtr expected_head_prev{nullptr, old_head.tag};
        TaggedPtr new_head_prev{node, old_head.tag + 1};
        if (!old_head.ptr->prev.compare_exchange_weak(expected_head_prev, new_head_prev)) {
          continue;
        }
      }

      TaggedPtr new_head{node, old_head.tag + 1};
      if (list.head.compare_exchange_weak(old_head, new_head)) {
        return;
      }
    }
  }


  void add_to_front(Node* node) noexcept {
    while (true) {
      TaggedPtr old_head = list.head.load();

      node->next.store({old_head.ptr, old_head.tag + 1});
      node->prev.store({nullptr, 0});

      if (old_head.ptr) {
        TaggedPtr expected_prev{nullptr, old_head.tag};
        TaggedPtr new_prev{node, old_head.tag + 1};
        if (!old_head.ptr->prev.compare_exchange_weak(expected_prev, new_prev)) {
          continue;
        }
      } else {
        list.tail.store({node, list.tail.load().tag + 1});
      }

      TaggedPtr new_head{node, old_head.tag + 1};
      if (list.head.compare_exchange_weak(old_head, new_head)) {
        list.size.fetch_add(1);
        return;
      }
    }
  }


  void remove() noexcept {
    while (true) {
      TaggedPtr tail = list.tail.load();
      if (!tail.ptr)
        return;

      TaggedPtr new_tail = tail.ptr->prev.load();

      if (new_tail.ptr) {
        TaggedPtr expected_next{tail.ptr, new_tail.tag};
        TaggedPtr new_next{nullptr, new_tail.tag + 1};
        if (!new_tail.ptr->next.compare_exchange_weak(expected_next, new_next)) {
          continue;
        }
      } else {
        list.head.store({nullptr, list.head.load().tag + 1});
      }

      TaggedPtr new_tail_tagged{new_tail.ptr, tail.tag + 1};
      if (list.tail.compare_exchange_weak(tail, new_tail_tagged)) {
        list.size.fetch_sub(1);

        MapPtr current_map = std::atomic_load(&map);
        MapPtr new_map     = std::make_shared<Map>(*current_map);
        new_map->erase(tail.ptr->key);

        if (std::atomic_compare_exchange_weak(&map, &current_map, new_map)) {
          delete tail.ptr;
          return;
        }
      }
    }
  }

private:
  NodeList          list;
  AtomicMapPtr      map;
  std::atomic<bool> is_shutting_down{false};
  std::size_t const capacity_;
};


}// namespace nut