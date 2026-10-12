#include "event.h"

#include <stdexcept>

namespace nano_edr
{
Event::Event(const EventParts& parts)
    : raw_ts_(parts.ts), type_(parts.type), pid_(parts.pid), fields_(parts.fields)
{
    if (raw_ts_.empty())
        throw std::invalid_argument("ts can not be empty");
    if (type_.empty())
        throw std::invalid_argument("type can not be empty");
    uint64_t temp;
    auto res = std::from_chars(parts.ts.data(),
                               parts.ts.data() + parts.ts.size(),
                               temp);
    if (res.ec != std::errc())
        throw std::invalid_argument("can not parse ts as uint64_t");
    if (res.ptr != parts.ts.data() + parts.ts.size())
        throw std::invalid_argument("size of ts > size of uint64_t");
    ts_ = Timestamp(temp);
}
}  // namespace nano_edr
