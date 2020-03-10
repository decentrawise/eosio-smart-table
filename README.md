# smart_table
An EOSIO smart-contract replacement for eosio::multi_index tables that implements caching, so that heavy computational smart-contracts that need to perform cyclic updates to many table records can do so in the cache, knowing that they will generate only the absolutely needed writes at the end to maintain state.

## Initial state

The start of this development had these results for a thousand table writes of an uint64_t field:
```bash
$ eoslime test

  Test profile contract
CPU: 354 NET: 184
    ✓ update John Doe profile (140ms)
CPU: 283 NET: 192
    ✓ update Jane Doe profile (185ms)
CPU: 12009 NET: 104
    ✓ John Doe count one thousand (228ms)
CPU: 14588 NET: 104
    ✓ Jane Doe count one thousand (213ms)

  4 passing (3s)
```
meaning that between 12ms and 14.5ms were being billed for the "count one thousand" action
CPU when using a multi_index table. These values come from the transaction receipt, so they
are the most accurate possible according to the eosio internal accounting.

## With smart-table

The idea is to make tables lazy, by using a `deque` to store the all entries and flush
the contents of of the cache when the object is destroyed, performing only one write to the
multi_index table per each entry in the cache that was modified.

You can check the code for this experiment in [smart_table](smart_table).

The final results from the same operations, added by another 10x the amount, are as follows:
```bash
$ eoslime test

  Test profile contract
CPU: 346 NET: 184

    ✓ update John Doe profile (114ms)
CPU: 408 NET: 192

    ✓ update Jane Doe profile (134ms)
CPU: 2961 NET: 104

    ✓ John Doe count one thousand (186ms)
CPU: 27497 NET: 104

    ✓ Jane Doe count ten thousand (151ms)

  4 passing (3s)
```

Please notice that for one thousand updates the difference in efficiency is around 5x (80%
saving on billed CPU), but it is also non linear, so for **ten thousand** (10x more) the total
CPU billed only grew by less than 9x.

### `smart_table` limitations

- only supports primary key, no other indexes (might be implemented soon)
- focus on `modify` optimization, so other operations like `emplace` are not optimized
- right now it fills all the internal cache all at once, might be interesting to
improve reading in chunks for large datasets

### Statistics helper wrappers

- smart_table_stats
- multi_index_stats
- singleton_stats

These wrapper provide operation counters for specific functionalities of each of the base classes,
so that we can measure what is going on with some intensive code and decide if it makes sense to use
smart_table or not.

## Conclusions

There's a lot of room to improve efficiency of smart contract code. `multi_index` tables are
expensive in CPU but are well optimized facing STL counterparts like `std::map`. Where we can
get more benefits in terms of efficiency is to minimize the writes by using internal structures
that do not require hashing/sorting, like `std::vector` or even better `std::deque`, if we do
not require traversing the data in a single contiguous data block.

### Possible future improvements (TBC)

- full test suite
- cache filling and flushing in chunks
- perform operations over current loaded chunk of data, using lambda for instance

## File tree

- src/ - source directory that contains `smart_table.hpp` main project code file

- src/stats/ - source directory that contains the statistics wrappers (check above)

- contracts/ - test contract directory

- tests/ - tests specs

- scripts/ - helper scripts for running nodeos locally


## Install EOSLime
```bash
$ npm install -g eoslime
```

## Compile the example contract
```bash
$ eoslime compile
```

## Run a local EOSIO node
```bash
$ ./scripts/nodeos.sh
```
**NOTE**: Please customize the script to your local development needs. This might be made
easier in the future with configuration and a better script...

## Run tests
```bash
$ eoslime test
```
