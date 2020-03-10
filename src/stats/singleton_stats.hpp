/**
 * A statistics wrapper for eosio::singleton containers
 */
#pragma once

#include "eosio/eosio.hpp"
#include "eosio/singleton.hpp"


template <eosio::name::raw SingletonName, typename T>
class singleton_stats : public eosio::singleton<SingletonName, T> {

private:

  typedef typename eosio::singleton<SingletonName, T> singleton;

  struct {
    uint64_t constructor = 0;
    uint64_t destructor = 0;
    uint64_t exists = 0;
    uint64_t get = 0;
    uint64_t set = 0;
    uint64_t remove = 0;
  } debug_stats;

  void print_debug_stats() {
    eosio::print("singleton debug statistics:\n");
    eosio::print("---------------------------\n");
    eosio::print_f("constructor: %\n", debug_stats.constructor);
    eosio::print_f("destructor: %\n", debug_stats.destructor);
    eosio::print_f("exists: %\n", debug_stats.exists);
    eosio::print_f("get: %\n", debug_stats.get);
    eosio::print_f("set: %\n", debug_stats.set);
    eosio::print_f("remove: %\n", debug_stats.remove);
  }

public:

  singleton_stats(eosio::name code, uint64_t scope) : singleton(code, scope) {
    ++debug_stats.constructor;
  }

  ~singleton_stats() {
    ++debug_stats.destructor;
    print_debug_stats();
  }

  bool exists() {
    ++debug_stats.exists;
    return singleton::exists();
  }

  T get() {
    ++debug_stats.get;
    return singleton::get();
  }

  T get_or_default( const T& def = T() ) {
    ++debug_stats.get;
    return singleton::get_or_default(def);
  }

  T get_or_create( eosio::name bill_to_account, const T& def = T() ) {
    ++debug_stats.get;
    return singleton::get_or_create(bill_to_account, def);
  }

  void set( const T& value, eosio::name bill_to_account ) {
    ++debug_stats.set;
    return singleton::set(value, bill_to_account);
  }

  void remove() {
    ++debug_stats.remove;
    singleton::remove();
  }

};
