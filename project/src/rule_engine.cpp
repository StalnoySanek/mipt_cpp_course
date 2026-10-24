#include "rule_engine.h"

namespace nano_edr
{
void RuleEngine::AddRule(std::unique_ptr<IRule> rule)
{
    rules_.push_back(std::move(rule));
}

std::size_t RuleEngine::ProcessEvent(const Event& event, std::vector<const IRule*>* fired)
{
    size_t cnt = 0;
    for (const auto& rule : rules_)
    {
        if (rule->Check(event))
        {
            ++cnt;
            if (fired != nullptr) fired->push_back(rule.get());
        }
    }
    return cnt;
}
}  // namespace nano_edr
