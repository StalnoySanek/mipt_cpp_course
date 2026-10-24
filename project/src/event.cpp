#include "event.h"

#include <charconv>
#include <format>
#include <stdexcept>

namespace nano_edr
{
Event::Event(const EventParts& parts)
    : raw_ts_(parts.ts), type_(parts.type), event_type_(EventTypeFromString(parts.type)), pid_(parts.pid), fields_(parts.fields)
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
        throw std::invalid_argument("can not parse ts as uint64_t. raw_ts=" + raw_ts_);
    if (res.ptr != parts.ts.data() + parts.ts.size())
        throw std::invalid_argument("size of ts > size of uint64_t. raw_ts=" + raw_ts_);
    ts_ = Timestamp(temp);
}

std::string ToString(const std::vector<Field>& fields)
{
    std::string res;
    for (auto& field : fields)
    {
        res += std::format("{}={} ", field.key, field.value);
    }
    return res;
}

std::string ToString(const Event& event)
{
    return std::format("ts={} type={} pid={} | {}",
                       event.raw_ts(), event.type(), event.pid(), ToString(event.fields()));
}

std::ostream& operator<<(std::ostream& out, const Event& event)
{
    return out << ToString(event);
}

EventType EventTypeFromString(const std::string& type)
{
    if (type == "process_start") return EventType::kProcessStart;
    if (type == "process_end") return EventType::kProcessEnd;
    if (type == "file_create") return EventType::kFileCreate;
    if (type == "file_write") return EventType::kFileWrite;
    if (type == "file_delete") return EventType::kFileDelete;
    if (type == "file_move") return EventType::kFileMove;
    if (type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;
}

}  // namespace nano_edr
