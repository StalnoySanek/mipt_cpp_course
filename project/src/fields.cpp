#include "fields.h"

#include <cctype>
#include <charconv>
#include <stdexcept>

namespace nano_edr
{

std::string to_lower(const std::string& s)
{
    std::string res = "";
    for (const auto& c : s)
        res += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return res;
}

const std::string* FindField(const Event& event, const std::string& key)
{
    for (const auto& field : event.fields)
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
    return event.type == "process_start";
}
bool IsFileWrite(const Event& event)
{
    return event.type == "file_write";
}
bool IsNetConnect(const Event& event)
{
    return event.type == "net_connect";
}

bool PathEndsWith(const Event& event, const std::string& suffix)
{
    const auto path = FindField(event, "path");
    if (!path)
        return false;
    return NormalizePath(*path).ends_with(to_lower(suffix));
}

bool CommandLineContains(const Event& event, const std::string& needle)
{
    const auto cmdline = FindField(event, "cmdline");
    if (!cmdline)
        return false;
    return to_lower(*cmdline).contains(to_lower(needle));
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

        res += c == '/' ? '\\' : c;
    }
    if (auto tmp = res.find("%TMP%"); tmp != std::string::npos)
        res.replace(tmp, 5, R"(\appdata\local\temp)");
    if (auto temp = res.find("%TEMP%"); temp != std::string::npos)
        res.replace(temp, 6, R"(\appdata\local\temp)");
    res = to_lower(res);
    return res;
}

}  // namespace nano_edr