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

            const Detection detection = MakeDetection(*rule, event);
            for (const auto& sink : sinks_)
                sink(detection);
        }
    }
    return cnt;
}

void RuleEngine::Subscribe(AlertSink sink) { sinks_.push_back(sink); }

Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const
{
    return Detection{
        .rule = rule.id(),
        .severity = rule.severity(),
        .ts = event.ts(),
        .pid = event.pid(),
        .chain = (model_ ? model_->DescribeChain(event.pid()) : ""),
        .actions = rule.actions(),
    };
}
}  // namespace nano_edr
