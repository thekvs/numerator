#include "disk_storage.hpp"

namespace numerator {

static const std::string kEmptyString = "";
static const size_t      kRestoreBatchSize = 10000;
static const int         kBloomFilterBits = 10;
static const uint32_t    kBinaryDumpMagicNumber = 0xdeadface;

void
DiskStorage::init(const std::string &path, int cache)
{
    cache_size = cache;

    options.create_if_missing = true;
    
    if (cache_size > 0) {
        // cache option is in megabytes
        options.block_cache = leveldb::NewLRUCache(1024 * 1024 * cache);
    }

    options.filter_policy = leveldb::NewBloomFilterPolicy(kBloomFilterBits);

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

    leveldb::ReadOptions read_options;

    if (cache_size < 0) {
        read_options.fill_cache = false;
    }

    BOOST_FOREACH(const Keys::value_type &k, keys) {
        leveldb::Slice key(reinterpret_cast<const char*>(&k), sizeof(k));
        status = db->Get(read_options, key, &value);
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
DiskStorage::dump(std::ostream &stream, bool binary_dump)
{
    NumID current_id;

    leveldb::ReadOptions read_options;

    read_options.fill_cache = false;
    boost::shared_ptr<leveldb::Iterator> it(db->NewIterator(read_options));

    if (binary_dump) {
        stream.write(reinterpret_cast<const char*>(&kBinaryDumpMagicNumber), sizeof(kBinaryDumpMagicNumber));
    }

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        THROW_EXC_IF_FAILED(it->status().ok(), "LevelDB iteration failed: %s", it->status().ToString().c_str());
        memcpy(&current_id, it->key().data(), it->key().size());
        if (binary_dump) {
            uint32_t value_size = it->value().ToString().size();
            stream.write(reinterpret_cast<const char*>(&current_id), sizeof(current_id));
            stream.write(reinterpret_cast<const char*>(&value_size), sizeof(value_size));
            stream.write(it->value().ToString().c_str(), value_size);
        } else {
            stream << current_id << "\t" << it->value().ToString() << std::endl;
        }
    }
}

void
DiskStorage::restore(std::ifstream &stream, bool binary_dump)
{
    THROW_EXC_IF_FAILED(db != NULL, "Database wasn't initialized");

    KVPairs kv_pairs;
    NumID   key;

    kv_pairs.reserve(kRestoreBatchSize);

    if (!binary_dump) {
        std::string line;
        Values      items;
        
        while (std::getline(stream, line)) {
            boost::split(items, line, boost::is_any_of("\t"));
            try {
                key = boost::lexical_cast<NumID>(items[0]);
            } catch (std::exception &exc) {
                THROW_EXC("%s at \"%s\"", exc.what(), line.c_str());
            }
            kv_pairs.push_back(KVPair(key, items[1]));
            if (kv_pairs.size() >= kRestoreBatchSize) {
                write(kv_pairs);
                kv_pairs.clear();
            }
        }
    } else {
        uint32_t magic_number = 0;
        uint32_t value_size = 0;

        stream.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        THROW_EXC_IF_FAILED(magic_number == kBinaryDumpMagicNumber, "Invalid binary dump magic number: %u (expected %u)",
            magic_number, kBinaryDumpMagicNumber);

        std::vector<char> buffer;
        std::string       value;

        while (!stream.eof()) {
            stream.read(reinterpret_cast<char*>(&key), sizeof(key));
            stream.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));
            buffer.reserve(value_size);
            stream.read(&buffer[0], value_size);
            value.assign(&buffer[0], value_size);

            kv_pairs.push_back(KVPair(key, value));
            if (kv_pairs.size() >= kRestoreBatchSize) {
                write(kv_pairs);
                kv_pairs.clear();
            }
        }
    }

    if (kv_pairs.size()) {
        write(kv_pairs);
    }
}

} // namespace
