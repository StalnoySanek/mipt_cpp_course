#include "conditions.h"

#include "fields.h"
#include "text.h"

namespace nano_edr
{
bool FieldEquals::Matches(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextEqualsIgnoreCase(*field, value_);
}

bool FieldContains::Matches(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextContainsIgnoreCase(*field, needle_);
}

bool FieldEndsWith::Matches(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    return field && TextEndsWithIgnoreCase(*field, suffix_);
}

bool FieldInList::Matches(const Event& event) const
{
    const std::string* field = FindField(event, key_);
    if (!field) return false;
    for (const auto& v : values_)
    {
        if (TextEqualsIgnoreCase(v, *field)) return true;
    }
    return false;
}

bool EventTypeIs::Matches(const Event& event) const
{
    for (auto type : types_)
    {
        if (type == event.event_type()) return true;
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition)
{
    parts_.push_back(std::move(condition));
}

bool AllOf::Matches(const Event& event) const
{
    for (const auto& cond : parts_)
    {
        if (!cond->Matches(event)) return false;
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition)
{
    parts_.push_back(std::move(condition));
}

bool AnyOf::Matches(const Event& event) const
{
    for (const auto& cond : parts_)
    {
        if (cond->Matches(event)) return true;
    }
    return false;
}

}  // namespace nano_edr