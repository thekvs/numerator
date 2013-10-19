#ifndef __DISK_STORAGE_HPP_INCLUDED__
#define __DISK_STORAGE_HPP_INCLUDED__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/cache.h>
#include <leveldb/filter_policy.h>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "gen-cpp/Numerator.h"
#include "exc.hpp"
#include "memory_storage.hpp"
#include "logger_initializer.hpp"

namespace numerator {

struct KVPair {
    NumID       key;
    std::string value;

    KVPair(): key(0) {}
    KVPair(NumID k, const std::string &v): key(k), value(v) {}
};

typedef std::vector<KVPair>      KVPairs;
typedef std::vector<NumID>       Keys;
typedef std::vector<std::string> Values;
typedef std::vector<FailureIdx>  Failures;

class DiskStorage {
public:

    DiskStorage():
        db(NULL),
        cache_size(0)
    {}

    ~DiskStorage()
    {
        delete db;
        delete options.block_cache;
        delete options.filter_policy;
    }

    void init(const std::string &path, int cache = 0);
    void write(const KVPairs &kv_pairs);
    NumID load_in_memory(MemoryStorage &storage);
    void lookup(const Keys &keys, Values &values, Failures &failures);
    void dump(std::ostream &stream);
    void restore(const std::string &data_file);

private:

    leveldb::DB      *db;
    int               cache_size;
    leveldb::Options  options;
};

} // namespace

#endif
