#include "rule.h"

namespace nano_edr
{
RuleBase::RuleBase(const std::string& id, Severity severity) : id_(id), severity_(severity) {}

void RuleBase::AddAction(const std::string& action)
{
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity) : RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition)
{
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event)
{
    for (const auto& cond : conditions_)
    {
        if (!cond->Matches(event))
            return false;
    }

    CountHit();
    return true;
}
SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms)
    : RuleBase(id, severity), window_ms_(window_ms) {}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> condition) { first_ = std::move(condition); }
void SequenceRule::SetSecond(std::unique_ptr<ICondition> condition) { second_ = std::move(condition); }

bool SequenceRule::Check(const Event& event)
{
    if (first_ == nullptr || second_ == nullptr) return false;
    std::vector<Pending> new_pending;
    for (const auto& p : pending_)
    {
        if (event.ts().ms > window_ms_ + p.ts_ms) continue;
        new_pending.push_back(p);
    }
    pending_ = std::move(new_pending);

    if (second_->Matches(event))
    {
        auto found = pending_.end();
        for (auto it = pending_.begin(); it != pending_.end(); ++it)
        {
            if (Linked(*it, event))
                found = it;
        }
        if (found != pending_.end())
        {
            pending_.erase(found);
            CountHit();
            return true;
        }
    }

    if (first_->Matches(event))
    {
        pending_.push_back({event.ts().ms, event.pid()});
    }
    return false;
}

LambdaRule::LambdaRule(const std::string& id, Severity severity, Predicate predicate)
    : RuleBase(id, severity), predicate_(std::move(predicate)) {}

bool LambdaRule::Check(const Event& event)
{
    if (!predicate_ || !predicate_(event))
        return false;

    CountHit();
    return true;
}

bool SequenceRule::Linked(const Pending& first, const Event& second) const
{
    if (child_of_prev_)
    {
        if (model_ == nullptr)
            return false;

        const auto* child = model_->FindProcess(second.pid());
        return child != nullptr && child->ppid == first.pid;
    }

    return !same_pid_ || first.pid == second.pid();
}
}  // namespace nano_edr