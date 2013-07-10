#ifndef __MEMORY_STORAGE_HPP_INCLUDED__
#define __MEMORY_STORAGE_HPP_INCLUDED__

#include <map>
#include <utility>
#include <stdint.h>

#include <Judy.h>
#include "exc.hpp"
#include "gen-cpp/Numerator.h"

namespace numerator {

/*typedef std::map<std::string, NumID> MemoryStorage;*/

class MemoryStorage {
public:

    MemoryStorage():
        data((Pvoid_t)NULL),
        value((PWord_t)NULL),
        ret((PWord_t)NULL)
    {
    }

    ~MemoryStorage()
    {
        long bytes;
        JSLFA(bytes, data);
    }

    std::pair<bool, NumID> find(const std::string &s, NumID &counter);
    void insert(const std::string &s, NumID id);

private:

    Pvoid_t  data;
    Word_t  *value;
    Word_t  *ret;
};

}

#endif
