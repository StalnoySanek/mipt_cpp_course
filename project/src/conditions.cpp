#include "conditions.h"

#include "fields.h"
#include "text.h"

namespace nano_edr
{
bool FieldEquals::operator()(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextEqualsIgnoreCase(*field, value_);
}

bool FieldContains::operator()(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextContainsIgnoreCase(*field, needle_);
}

bool FieldEndsWith::operator()(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextEndsWithIgnoreCase(*field, suffix_);
}

bool FieldInList::operator()(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    if (!field) return false;
    for (const auto& v : values_)
    {
        if (TextEqualsIgnoreCase(v, *field)) return true;
    }
    return false;
}

Condition Condition::Equals(const std::string& key,
                            const std::vector<std::string>& values)
{
    return Condition(Condition::Match::kEquals, key, values);
}

Condition Condition::Contains(const std::string& key,
                              const std::vector<std::string>& values)
{
    return Condition(Condition::Match::kContains, key, values);
}

Condition Condition::EndsWith(const std::string& key,
                              const std::vector<std::string>& values)
{
    return Condition(Condition::Match::kEndsWith, key, values);
}

bool Condition::operator()(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    if (!field) return false;
    for (const auto& v : values_)
    {
        switch (match_)
        {
            case Condition::Match::kEquals:
                if (TextEqualsIgnoreCase(v, *field)) return true;
                break;
            case Condition::Match::kContains:
                if (TextContainsIgnoreCase(*field, v)) return true;
                break;
            case Condition::Match::kEndsWith:
                if (TextEndsWithIgnoreCase(*field, v)) return true;
                break;
        }
    }
    return false;
}

}  // namespace nano_edr