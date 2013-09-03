#ifndef __DISK_STORAGE_HPP_INCLUDED__
#define __DISK_STORAGE_HPP_INCLUDED__

#include <string>
#include <vector>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/cache.h>

#include <boost/foreach.hpp>

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

    DiskStorage(): db(NULL) {}

    ~DiskStorage()
    {
        delete db;
        delete options.block_cache;
    }

    void init(const std::string &path, size_t cache = 0);
    void write(const KVPairs &kv_pairs);
    NumID load_in_memory(MemoryStorage &storage);
    void lookup(const Keys &keys, Values &values, Failures &failures);

private:

    leveldb::DB      *db;
    leveldb::Options  options;
};

} // namespace

#endif
