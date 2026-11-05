#include "fields.h"

#include <cctype>
#include <charconv>
#include <stdexcept>

#include "text.h"

namespace nano_edr
{

char to_lower(const char c)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

void replace_all(std::string& s, const std::string& from, const std::string& to)
{
    auto pos = s.find(from);
    while (pos != std::string::npos)
    {
        s.replace(pos, from.size(), to);
        pos = s.find(from);
    }
}

const std::string* FindField(const Event& event, const std::string& key)
{
    if (key == "ts")
        return &event.raw_ts();
    if (key == "type")
        return &event.type();
    if (key == "pid")
        return &event.pid();
    for (const auto& field : event.fields())
    {
        if (field.key == key)
        {
            return &field.value;
        }
    }
    return nullptr;
}
const std::string& GetRequiredField(const Event& event, const std::string& key)
{
    const std::string* field = FindField(event, key);
    if (!field)
        throw std::invalid_argument("Can not find required key: " + key);
    return *field;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out)
{
    const std::string* field = FindField(event, key);
    if (!field)
        return false;
    const char* begin = field->data();
    const char* end = begin + field->size();
    auto res = std::from_chars(begin, end, *out);
    return res.ec == std::errc() && res.ptr == end;
}

uint64_t GetIntField(const Event& event, const std::string& key,
                     uint64_t fallback)
{
    uint64_t out;
    return GetIntField(event, key, &out) ? out : fallback;
}

bool IsProcessStart(const Event& event)
{
    return event.type() == "process_start";
}
bool IsFileWrite(const Event& event)
{
    return event.type() == "file_write";
}
bool IsNetConnect(const Event& event)
{
    return event.type() == "net_connect";
}

bool PathEndsWith(const Event& event, const std::string& suffix)
{
    const auto path = FindField(event, "path");
    if (!path)
        return false;
    return TextEndsWithIgnoreCase(NormalizePath(*path), suffix);
}

bool CommandLineContains(const Event& event, const std::string& needle)
{
    const auto cmdline = FindField(event, "cmdline");
    if (!cmdline)
        return false;
    return TextContainsIgnoreCase(*cmdline, needle);
}

std::string NormalizePath(const std::string& path)
{
    std::string res = "";
    for (const auto& c : path)
    {
        if (!res.empty() &&
            (c == '\\' || c == '/') &&
            res.back() == '\\')
        {
            continue;
        }

        res += c == '/' ? '\\' : to_lower(c);
    }
    // res = to_lower(res);
    replace_all(res, "%tmp%", R"(\appdata\local\temp)");
    replace_all(res, "%temp%", R"(\appdata\local\temp)");
    return res;
}

}  // namespace nano_edr