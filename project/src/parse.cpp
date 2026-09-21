#include "parse.h"

const char* ws = " \t";

namespace nano_edr
{
bool IsBlankOrComment(const std::string* line)
{
    if (line->empty())
        return true;

    size_t pos = line->find_first_not_of(ws);
    if (pos == std::string::npos)
        return true;

    return (*line)[pos] == '#' || (*line)[pos] == ';';
}

bool _is_space(char c)
{
    return std::string_view(ws).find(c) != std::string::npos;
}

bool _parse_key(const std::string* line, size_t* pos, std::string* out)
{
    *out = "";
    while ((*line)[*pos] != '=')
    {
        if (_is_space((*line)[*pos]))
            return false;
        if (*pos == line->length())
            return false;
        *out += (*line)[*pos];
        ++*pos;
    }
    ++*pos;
    return !out->empty();
}

bool _parse_value(const std::string* line, size_t* pos, std::string* out)
{
    *out = "";
    bool IsPath = false;
    while (true)
    {
        char c = (*line)[*pos];
        if (!IsPath && _is_space(c))
            break;
        if (*pos == line->length())
            break;
        if (!IsPath && c == '"')
        {
            if (!out->empty())
                return false;
            IsPath = true;
            ++*pos;
            continue;
        }
        if (IsPath && c == '"')
        {
            ++*pos;
            return true;
        }
        *out += c;
        ++*pos;
    }

    if (IsPath)
        return false;
    return true;
}

bool ParseEventLine(const std::string* line, Event* out)
{
    *out = Event();
    if (IsBlankOrComment(line))
        return false;

    std::string key = "";
    std::string value = "";
    std::vector<Field> fields;
    size_t pos = line->find_first_not_of(ws);
    while (pos < line->length())
    {
        if (!_parse_key(line, &pos, &key) || !_parse_value(line, &pos, &value))
            return false;
        while (pos < line->length() && _is_space((*line)[pos]))
            ++pos;
        fields.emplace_back(key, value);
    }
    for (auto& [k, v] : fields)
    {
        if (k == "ts" && out->ts.empty())
            out->ts = v;
        else if (k == "type" && out->type.empty())
            out->type = v;
        else if (k == "pid" && out->pid.empty())
            out->pid = v;
        else
            out->fields.emplace_back(k, v);
    }
    if (out->ts.empty() || out->type.empty())
        return false;
    return true;
}
}  // namespace nano_edr