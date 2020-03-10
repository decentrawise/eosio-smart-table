/**
 * A statistics wrapper for eosio::multi_index tables
 */
#pragma once

#include "eosio/eosio.hpp"


template <eosio::name::raw TableName, typename T>
class multi_index_stats : public eosio::multi_index<TableName, T> {

private:

  typedef typename eosio::multi_index<TableName, T> table;
  typedef typename eosio::multi_index<TableName, T>::const_iterator table_iterator;

  mutable struct {
    uint64_t constructor = 0;
    uint64_t destructor = 0;
    uint64_t emplace = 0;
    uint64_t modify = 0;
    uint64_t get = 0;
    uint64_t find = 0;
    uint64_t erase = 0;
  } debug_stats;

  void print_debug_stats() {
    eosio::print("multi_index debug statistics:\n");
    eosio::print("-----------------------------\n");
    eosio::print_f("constructor: %\n", debug_stats.constructor);
    eosio::print_f("destructor: %\n", debug_stats.destructor);
    eosio::print_f("emplace: %\n", debug_stats.emplace);
    eosio::print_f("modify: %\n", debug_stats.modify);
    eosio::print_f("get: %\n", debug_stats.get);
    eosio::print_f("find: %\n", debug_stats.find);
    eosio::print_f("erase: %\n", debug_stats.erase);
  }

public:

  multi_index_stats(eosio::name code, uint64_t scope) : table(code, scope) {
    ++debug_stats.constructor;
  }

  ~multi_index_stats() {
    ++debug_stats.destructor;
    print_debug_stats();
  }

  template<typename Lambda>
  table_iterator emplace(eosio::name payer, Lambda&& constructor) {
    ++debug_stats.emplace;
    return table::emplace(payer, constructor);
  }

  template <typename Lambda>
  void modify(table_iterator itr, eosio::name payer, Lambda&& updater) {
    ++debug_stats.modify;
    table::modify(itr, payer, updater);
  }

  template<typename Lambda>
  void modify( const T& obj, eosio::name payer, Lambda&& updater ) {
    ++debug_stats.modify;
    table::modify(obj, payer, updater);
  }

  const T& get(uint64_t primary, const char* error_msg = "unable to find key" ) const {
    ++debug_stats.get;
    return table::get(primary, error_msg);
  }

  table_iterator find(uint64_t primary) const {
    ++debug_stats.find;
    return table::find(primary);
  }

  table_iterator require_find(uint64_t primary, const char* error_msg = "unable to find key") const {
    ++debug_stats.find;
    return table::require_find(primary, error_msg);
  }

  table_iterator erase(table_iterator itr) {
    ++debug_stats.erase;
    return table::erase(itr);
  }

  void erase( const T& obj ) {
    ++debug_stats.erase;
    return table::erase(obj);
  }

};
