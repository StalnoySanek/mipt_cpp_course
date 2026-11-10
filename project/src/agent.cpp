#include "agent.h"

#include <algorithm>
#include <print>
#include <stdexcept>

#include "agent_rules.h"
#include "parse.h"
#include "rules.h"
namespace nano_edr
{
Agent::Agent(bool quiet)
    : quiet_(quiet), types_(), engine_(BuildAgentRules()) {}

void Agent::HandleEvent(const Event& event)
{
    ++cnt_;
    ++types_[event.type()];
    std::vector<const IRule*> fired;
    size_t detected = engine_.ProcessEvent(event, &fired);

    if (detected && !quiet_) PrintCtx();

    for (const auto& rule : fired)
    {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()),
                   rule->id(),
                   event.raw_ts(),
                   event.pid());
    }

    window_.PushBack(WindowEntry{.ts = event.ts(),
                                 .type = event.type(),
                                 .pid = event.pid()});
}

void Agent::PrintCtx()
{
    if (window_.empty()) return;

    if (window_.size() > 1)
    {
        const auto& entry = window_[window_.size() - 2];
        std::print("[CTX] -2: {} {} pid={}\n", entry.ts.ms, entry.type, entry.pid);
    }

    const auto& entry = window_.back();
    std::print("[CTX] -1: {} {} pid={}\n", entry.ts.ms, entry.type, entry.pid);
}

void Agent::PrintSummary() const
{
    if (quiet_) return;
    std::print("Обработано {} строк. Из них:\n\tСломано -> {}\n\tПропущено -> {}\n\tОбработано -> {}\n",
               broken_ + skipped_ + cnt_, broken_, skipped_, cnt_);
    std::print("Всего обнаружено {} событий:\n", cnt_);
    for (auto& [type, count] : types_)
    {
        std::print("\t{} -> {}\n", type, count);
    }
    std::print("\n\nДетекты по правилам:");
    for (size_t i = 0; i < engine_.size(); ++i)
    {
        std::print("\t{} -> {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
}

void Agent::OnTick() {}
void Agent::OnSkipped() { ++skipped_; }
void Agent::OnBroken() { ++broken_; }
}  // namespace nano_edr