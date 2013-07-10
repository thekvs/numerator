#include "memory_storage.hpp"

namespace numerator {

std::pair<bool, NumID>
MemoryStorage::find(const std::string &s, NumID &counter)
{
    NumID id = 0;
    bool  inserted = false;

    ret = (PWord_t)JudySLGet(data, reinterpret_cast<const uint8_t*>(s.c_str()), PJE0);
    if (ret != NULL) {
        id = *ret;
    } else {
        value = (PWord_t)JudySLIns(&data, reinterpret_cast<const uint8_t*>(s.c_str()), PJE0);
        THROW_EXC_IF_FAILED(value != PJERR, "Memory allocation failed");
        *value = counter++;
        id = *value;
        inserted = true;
    }

    return std::make_pair(inserted, id);
}

void
MemoryStorage::insert(const std::string &s, NumID id)
{
    value = (PWord_t)JudySLGet(data, reinterpret_cast<const uint8_t*>(s.c_str()), PJE0);
    if (value == NULL) {
        value = (PWord_t)JudySLIns(&data, reinterpret_cast<const uint8_t*>(s.c_str()), PJE0);
        THROW_EXC_IF_FAILED(value != PJERR, "Memory allocation failed");
        *value = id;
    } else {
        *value = id;
    }
}

} // namespace
