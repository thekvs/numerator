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
        value((PWord_t)NULL)
    {
    }

    ~MemoryStorage()
    {
        unload();
    }

    bool empty() const {
        return (data == (Pvoid_t)NULL);
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
