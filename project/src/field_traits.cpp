#include "field_traits.h"

#include "text.h"

namespace nano_edr
{

bool FieldTraits<std::string>::Parse(const std::string& data, std::string* out)
{
    *out = data;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& data, uint64_t* out)
{
    uint64_t temp = 0;
    auto res = std::from_chars(data.data(), data.data() + data.size(), temp);

    if (res.ec != std::errc() || res.ptr != data.data() + data.size())
        return false;

    *out = temp;
    return true;
}
bool FieldTraits<bool>::Parse(const std::string& data, bool* out)
{
    if (data == "1" || TextEqualsIgnoreCase(data, "true") || TextEqualsIgnoreCase(data, "yes"))
    {
        *out = true;
        return true;
    }

    if (data == "0" || TextEqualsIgnoreCase(data, "false") || TextEqualsIgnoreCase(data, "no"))
    {
        *out = false;
        return true;
    }
    return false;
}

}  // namespace nano_edr
