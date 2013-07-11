#include "memory_storage.hpp"

namespace numerator {

std::pair<bool, NumID>
MemoryStorage::find(const std::string &s, NumID &counter)
{
    NumID id = 0;
    bool  inserted = false;

    JError_t err;

    memset(&err, 0, sizeof(err));
    PPvoid_t rv = JudySLGet(data, reinterpret_cast<const uint8_t*>(s.c_str()), &err);
    THROW_EXC_IF_FAILED(JU_ERRNO(&err) == 0 && rv != PPJERR,
        "Memory operaton failed, error code: %i", JU_ERRNO(&err));

    if (rv != NULL) {
        id = *reinterpret_cast<NumID*>(rv);
    } else {
        memset(&err, 0, sizeof(err));
        rv = JudySLIns(&data, reinterpret_cast<const uint8_t*>(s.c_str()), &err);
        THROW_EXC_IF_FAILED(JU_ERRNO(&err) == 0 && rv != NULL && rv != PPJERR,
            "Memory allocation failed, error code: %i", JU_ERRNO(&err));
        value = reinterpret_cast<PWord_t>(rv);
        id = counter++;
        *value = id;
        inserted = true;
    }

    return std::make_pair(inserted, id);
}

void
MemoryStorage::insert(const std::string &s, NumID id)
{
    JError_t err;

    memset(&err, 0, sizeof(err));
    PPvoid_t rv = JudySLGet(data, reinterpret_cast<const uint8_t*>(s.c_str()), &err);
    THROW_EXC_IF_FAILED(JU_ERRNO(&err) == 0 && rv != PPJERR,
        "Memory operaton failed, error code: %i", JU_ERRNO(&err));

    if (rv == NULL) {
        memset(&err, 0, sizeof(err));
        rv = JudySLIns(&data, reinterpret_cast<const uint8_t*>(s.c_str()), &err);
        THROW_EXC_IF_FAILED(JU_ERRNO(&err) == 0 && rv != NULL && rv != PPJERR,
            "Memory allocation failed, error code: %i", JU_ERRNO(&err));
    }

    value = reinterpret_cast<PWord_t>(rv);
    *value = id;
}

} // namespace
