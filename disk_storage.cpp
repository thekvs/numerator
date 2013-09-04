#include "disk_storage.hpp"

namespace numerator {

static const std::string kEmptyString = "";

void
DiskStorage::init(const std::string &path, size_t cache)
{
    options.create_if_missing = true;
    
    if (cache > 0) {
        // cache option is in megabytes
        options.block_cache = leveldb::NewLRUCache(1024 * 1024 * cache);
    }

    leveldb::Status status = leveldb::DB::Open(options, path, &db);
    THROW_EXC_IF_FAILED(status.ok(), "LevelDB initialization failed: %s", status.ToString().c_str());
}

void
DiskStorage::write(const KVPairs &kv_pairs)
{
    leveldb::WriteBatch batch;
    leveldb::Status     status;

    BOOST_FOREACH(const KVPairs::value_type &p, kv_pairs) {
        leveldb::Slice key(reinterpret_cast<const char*>(&p.key), sizeof(p.key));
        batch.Put(key, p.value);
    }

    status = db->Write(leveldb::WriteOptions(), &batch);
    THROW_EXC_IF_FAILED(status.ok(), "LevelDB's write method failed: %s", status.ToString().c_str());
}

NumID
DiskStorage::load_in_memory(MemoryStorage &storage)
{
    NumID current_id;
    NumID max_id = 0;

    size_t loaded = 0;

    leveldb::ReadOptions read_options;

    read_options.fill_cache = false;
    boost::shared_ptr<leveldb::Iterator> it(db->NewIterator(read_options));

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        THROW_EXC_IF_FAILED(it->status().ok(), "LevelDB iteration failed: %s", it->status().ToString().c_str());
        memcpy(&current_id, it->key().data(), it->key().size());
        storage.insert(it->value().ToString(), current_id);
        loaded++;
        if (current_id > max_id) {
            max_id = current_id;
        }
    }

    LOG(INFO) << "loaded " << loaded << " entries";
    
    return max_id;
}

void
DiskStorage::lookup(const Keys &keys, Values &values, Failures &failures)
{
    leveldb::Status    status;
    Values::value_type value;

    values.clear();
    failures.clear();

    FailureIdx failure_idx = 0;

    BOOST_FOREACH(const Keys::value_type &k, keys) {
        leveldb::Slice key(reinterpret_cast<const char*>(&k), sizeof(k));
        status = db->Get(leveldb::ReadOptions(), key, &value);
        if (status.ok()) {
            values.push_back(value);
        } else if (status.IsNotFound()) {
            failures.push_back(failure_idx);
            values.push_back(kEmptyString);
        } else {
            THROW_EXC("LevelDB's Get method failed: %s", status.ToString().c_str());
        }
        failure_idx++;
    }
}

void
DiskStorage::dump(std::ostream &stream)
{
    NumID current_id;

    leveldb::ReadOptions read_options;

    read_options.fill_cache = false;
    boost::shared_ptr<leveldb::Iterator> it(db->NewIterator(read_options));

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        THROW_EXC_IF_FAILED(it->status().ok(), "LevelDB iteration failed: %s", it->status().ToString().c_str());
        memcpy(&current_id, it->key().data(), it->key().size());
        stream << current_id << "\t" << it->value().ToString() << std::endl;
    }
}

} // namespace
