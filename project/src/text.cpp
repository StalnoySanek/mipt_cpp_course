#include "text.h"

#include <cctype>

std::string to_lower(const std::string& s)
{
    std::string res = "";
    for (const auto& c : s)
        res += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return res;
}

bool TextEqualsIgnoreCase(const std::string& left, const std::string& right)
{
    return to_lower(left) == to_lower(right);
}

bool TextContainsIgnoreCase(const std::string& text, const std::string& needle)
{
    return to_lower(text).contains(to_lower(needle));
}

bool TextEndsWithIgnoreCase(const std::string& text, const std::string& suffix)
{
    return to_lower(text).ends_with(to_lower(suffix));
}