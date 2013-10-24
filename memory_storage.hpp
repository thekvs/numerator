#ifndef __MEMORY_STORAGE_HPP_INCLUDED__
#define __MEMORY_STORAGE_HPP_INCLUDED__

#include <map>
#include <utility>
#include <stdint.h>

#include <Judy.h>
#include "exc.hpp"
#include "gen-cpp/Numerator.h"

namespace numerator {

class MemoryStorage {
public:

    MemoryStorage():
        data(NULL),
        value(NULL)
    {
    }

    ~MemoryStorage()
    {
        unload();
    }

    bool empty() const {
        return (data == NULL);
    }

    void unload()
    {
        long bytes;
        JSLFA(bytes, data);
        data = NULL;   
    }

    std::pair<bool, NumID> find(const std::string &s, NumID &counter);
    void insert(const std::string &s, NumID id);

private:

    Pvoid_t  data;
    Word_t  *value;
};

}

#endif
