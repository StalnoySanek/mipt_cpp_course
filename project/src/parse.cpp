#include "parse.h"

const char* ws = " \t";

namespace nano_edr
{
bool IsBlankOrComment(const std::string& line)
{
    if (line.empty())
        return true;

    size_t pos = line.find_first_not_of(ws);
    if (pos == std::string::npos)
        return true;

    return line[pos] == '#' || line[pos] == ';';
}

bool is_space(char c)
{
    return std::string_view(ws).contains(c);
}

bool parse_key(const std::string& line, size_t* pos, std::string* out)
{
    *out = "";
    while (*pos < line.length())
    {
        if (line[*pos] == '=')
        {
            ++*pos;
            return !out->empty();
        }

        // space in key is invalid
        if (is_space(line[*pos]))
            return false;

        *out += line[*pos];
        ++*pos;
    }

    // end line after key is invalid
    return false;
}

bool parse_value(const std::string& line, size_t* pos, std::string* out)
{
    *out = "";
    bool IsPath = false;
    while (*pos < line.length())
    {
        char c = line[*pos];

        // space is end of value (if not a path)
        if (!IsPath && is_space(c))
            break;

        if (!IsPath && c == '"')
        {
            // characters befor `"` are invalid
            if (!out->empty())
                return false;
            IsPath = true;
            ++*pos;
            continue;
        }
        if (IsPath && c == '"')
        {
            ++*pos;
            // characters after `"` are invalid
            return *pos >= line.length() || is_space(line[*pos]);
        }
        *out += c;
        ++*pos;
    }

    return !IsPath;
}

bool ParseEventParts(const std::string& line, EventParts* out)
{
    *out = EventParts();
    if (IsBlankOrComment(line))
        return false;

    std::string key = "";
    std::string value = "";
    size_t pos = line.find_first_not_of(ws);
    while (pos < line.length())
    {
        if (!parse_key(line, &pos, &key) || !parse_value(line, &pos, &value))
            return false;
        while (pos < line.length() && is_space(line[pos]))
            ++pos;
        if (key == "ts" && out->ts.empty())
            out->ts = value;
        else if (key == "type" && out->type.empty())
            out->type = value;
        else if (key == "pid" && out->pid.empty())
            out->pid = value;
        else
            out->fields.emplace_back(key, value);
    }
    return true;
}
}  // namespace nano_edr