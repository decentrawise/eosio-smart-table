/**
 * A smart_table is a caching wrapper for eosio::multi_index tables
 */
#pragma once

#include <deque>

#include "eosio/eosio.hpp"


template <eosio::name::raw TableName, typename T>
class smart_table : public eosio::multi_index<TableName, T> {

private:

  typedef typename eosio::multi_index<TableName, T> table;
  typedef typename eosio::multi_index<TableName, T>::const_iterator table_iterator;

  typedef struct {
    table_iterator titr;
    eosio::name payer;
    T entry;
  } cache_entry;

  mutable std::deque<cache_entry> cache;
  const std::deque<cache_entry>& const_cache = cache;

protected:

  mutable bool filled = false;

  int fill() const {
    if (filled) return -1;

    int counter = 0;
    for (auto titr = table::begin(); titr != table::end(); ++titr) {
      cache_entry elem{titr, eosio::name(), *titr};
      cache.push_back(elem);
      ++counter;
    }
    filled = true;

    return counter;
  }

  virtual inline void fill_if_needed() const { if (!filled) fill(); }

  int flush() {
    if (!filled) return -1;

    int counter = 0;
    for (auto elem: cache) {
      if (elem.payer != eosio::name()) {
        table::modify(elem.titr, elem.payer, [&](auto& obj) {
          obj = elem.entry;
        });
        ++counter;
      }
    }
    cache.clear();
    filled = false;

    return counter;
  }

public:

  template <typename Base = typename std::deque<cache_entry>::const_iterator>
  struct cache_iterator : public Base {
    const T& operator*() const { return Base::operator*().entry; }
    const T* operator->() const { return &Base::operator*().entry; }

  private:
    cache_entry& _entry() { return const_cast<cache_entry&>(Base::operator*()); }

    cache_iterator(const Base& itr) : Base(itr) {}

    friend class smart_table;
  };
  typedef cache_iterator<> const_iterator;

  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  smart_table(eosio::name code, uint64_t scope) : table(code, scope) { }

  ~smart_table() {
    flush();
  }

  const_iterator cbegin() const { fill_if_needed(); return const_cache.cbegin(); }
  const_iterator begin() const { fill_if_needed(); return const_cache.begin(); }
  const_iterator cend() const { fill_if_needed(); return const_cache.cend(); }
  const_iterator end() const { fill_if_needed(); return const_cache.end(); }
  const_reverse_iterator crbegin() const { fill_if_needed(); return const_cache.crbegin(); }
  const_reverse_iterator rbegin() const { fill_if_needed(); return const_cache.rbegin(); }
  const_reverse_iterator crend() const { fill_if_needed(); return const_cache.crend(); }
  const_reverse_iterator rend() const { fill_if_needed(); return const_cache.rend(); }

  const_iterator lower_bound(uint64_t primary) const {
    fill_if_needed();
    auto itr = table::lower_bound(primary);
    return find_in_cache(itr);
  }

  const_iterator upper_bound( uint64_t primary ) const {
    fill_if_needed();
    auto itr = table::upper_bound(primary);
    return find_in_cache(itr);
  }

  template<eosio::name::raw IndexName>
  auto get_index() {
    eosio::check(false, "smart_table doesn't allow use of secondary indexes");
  }

  template<eosio::name::raw IndexName>
  auto get_index() const {
    eosio::check(false, "smart_table doesn't allow use of secondary indexes");
  }

  const_iterator iterator_to(const T& obj) const {
    fill_if_needed();
    auto pk = obj.primary_key();
    auto titr = table::find(pk);
    if (titr == table::end()) return end();
    return find_in_cache(titr);
  }

  template<typename Lambda>
  const_iterator emplace(eosio::name payer, Lambda&& constructor) {
    fill_if_needed();
    auto titr = table::emplace(payer, constructor);
    cache_entry elem{titr, eosio::name(), *titr};
    cache.push_back(elem);
    return find_in_cache(titr);
  }

  template <typename Lambda>
  void modify(const_iterator itr, eosio::name payer, Lambda&& updater) {
    fill_if_needed();
    eosio::check(itr != end(), "cannot pass end iterator to modify");
    auto& obj = itr._entry();

    // Do not forget the auto& otherwise it would make a copy and thus not update at all.
    auto& mutableobj = const_cast<T&>(obj.entry);
    updater(mutableobj);

    // Update
    obj.payer = payer;
    obj.entry = mutableobj;
  }

  template<typename Lambda>
  void modify( const T& obj, eosio::name payer, Lambda&& updater ) {
    modify(iterator_to(obj), payer, updater);
  }

  const T& get(uint64_t primary, const char* error_msg = "unable to find key" ) const {
    fill_if_needed();
    auto result = find(primary);
    eosio::check(result != cend(), error_msg);
    return *result;
  }

  const_iterator find(uint64_t primary) const {
    fill_if_needed();
    auto titr = table::find(primary);
    if (titr == table::end()) return end();
    return find_in_cache(titr);
  }

  const_iterator require_find(uint64_t primary, const char* error_msg = "unable to find key") const {
    auto itr = find(primary);
    eosio::check(itr != end(), error_msg);
    return itr;
  }

  const_iterator erase(const_iterator itr) {
    fill_if_needed();
    eosio::check(itr != end(), "cannot pass end iterator to erase");
    table::erase(itr._entry().titr);
    auto next = itr + 1;
    cache.erase(itr);
    return next;
  }

  void erase( const T& obj ) {
    fill_if_needed();
    auto itr = iterator_to(obj);
    eosio::check(itr != end(), "object passed to erase was not found");
    table::erase(obj);
    cache.erase(itr);
  }

private:

  const_iterator find_in_cache(table_iterator& titr) const {
    // DEBUG_PRINT("smart_table::find_in_cache")
    for (auto itr = cache.begin(); itr != cache.end(); ++itr) {
      if ((*itr).titr == titr) return const_iterator(itr);
    }
    return end();
  }

};
